#include "space.hpp"


OctreeNode * OctreeAllocator::allocateOctreeNode(BoundingCube &cube){
    OctreeNode * node = nodeAllocator.allocate();
    mtx.lock();
    compactMap.try_emplace(cube, node);
    mtx.unlock();
    return node;
}

OctreeNode * OctreeAllocator::getOctreeNode(BoundingCube &cube) {
    mtx.lock();
    auto it = compactMap.find(cube);
    if (it != compactMap.end()) {
        mtx.unlock();
        return it->second;
    }
    mtx.unlock();
    return NULL;
}

OctreeNode * OctreeAllocator::getOctreeNode(uint index){
    return nodeAllocator.getFromIndex(index);
}

void OctreeAllocator::deallocateOctreeNode(OctreeNode * node, BoundingCube &cube){
    mtx.lock();
    compactMap.erase(cube);
    mtx.unlock();
    nodeAllocator.deallocate(node);
}

uint OctreeAllocator::getIndex(OctreeNode * node){
    return nodeAllocator.getIndex(node);
}

size_t OctreeAllocator::getBlockSize() const {
    return nodeAllocator.getBlockSize();   
}

size_t OctreeAllocator::getAllocatedBlocksCount() {
    return nodeAllocator.getAllocatedBlocksCount();    
}

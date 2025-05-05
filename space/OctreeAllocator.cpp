#include "space.hpp"


OctreeNode * OctreeAllocator::allocateOctreeNode(BoundingCube &cube){
    OctreeNode * node = nodeAllocator.allocate();
    compactMap.try_emplace(cube, node);
    return node;
}

OctreeNode * OctreeAllocator::getOctreeNode(uint index){
    return nodeAllocator.getFromIndex(index);
}

void OctreeAllocator::deallocateOctreeNode(OctreeNode * node, BoundingCube &cube){
    compactMap.erase(cube);
    nodeAllocator.deallocate(node);
}

uint OctreeAllocator::getIndex(OctreeNode * node){
    return nodeAllocator.getIndex(node);
}

size_t OctreeAllocator::getBlockSize() const {
    return nodeAllocator.getBlockSize();   
}

size_t OctreeAllocator::getAllocatedBlocksCount() const {
    return nodeAllocator.getAllocatedBlocksCount();    
}

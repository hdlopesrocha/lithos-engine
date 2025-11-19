#include "space.hpp"


OctreeNode * OctreeAllocator::allocate(){
    OctreeNode * result = nodeAllocator.allocate();
    return result;
}

void OctreeAllocator::get(OctreeNode * nodes[8], uint indices[8]){
    nodeAllocator.getFromIndices(nodes, indices);
}

OctreeNode * OctreeAllocator::get(uint index){
    return nodeAllocator.getFromIndex(index);
}

OctreeNode * OctreeAllocator::deallocate(OctreeNode * node){
    nodeAllocator.deallocate(node);
    return NULL;
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

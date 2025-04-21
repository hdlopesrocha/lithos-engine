#include "space.hpp"

ChildBlock::ChildBlock() {
    for(int i=0; i < 8 ; ++i) {
        children[i] = UINT_MAX;
    }
}

ChildBlock * ChildBlock::init() {
    for(int i=0; i < 8 ; ++i) {
        children[i] = UINT_MAX;
    }
    return this;
}

void ChildBlock::clear(OctreeAllocator * allocator) {
    for(int i=0; i < 8 ; ++i) {
        OctreeNode * child = allocator->nodeAllocator.getFromIndex(this->children[i]);
        if(child != NULL) {
            child->clear(allocator);
            allocator->nodeAllocator.deallocate(child);
            children[i] = UINT_MAX;
        }
    }
}
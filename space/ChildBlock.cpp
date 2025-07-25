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

void ChildBlock::clear(OctreeAllocator * allocator, BoundingCube &cube, OctreeChangeHandler * handler) {
    for(int i=0; i < 8 ; ++i) {
        OctreeNode * child = allocator->getOctreeNode(this->children[i]);
        if(child != NULL) {
            child->clear(allocator, cube, handler);
            BoundingCube subCube = cube.getChild(i);
            allocator->deallocateOctreeNode(child, subCube);
            children[i] = UINT_MAX;
        }
    }
}

void ChildBlock::set(int i, OctreeNode * node, OctreeAllocator * allocator) {
    children[i] = allocator->getIndex(node);
}

OctreeNode * ChildBlock::get(int i, OctreeAllocator * allocator){
    return allocator->getOctreeNode(children[i]);
}
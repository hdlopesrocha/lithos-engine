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

void ChildBlock::clear(OctreeAllocator &allocator, BoundingCube &cube, OctreeChangeHandler * handler) {
    for(int i=0; i < 8 ; ++i) {
        OctreeNode * child = allocator.getOctreeNode(this->children[i]);
        if(child != NULL) {
            BoundingCube subCube = cube.getChild(i);
            ChildBlock * childBlock = child->getBlock(allocator);
            child->clear(allocator, subCube, handler, childBlock);
            allocator.deallocateOctreeNode(child, cube);
        }
        children[i] = UINT_MAX;
    }
}


void ChildBlock::set(int i, OctreeNode * node, OctreeAllocator * allocator) {
    children[i] = allocator->getIndex(node);
}

OctreeNode * ChildBlock::get(int i, OctreeAllocator * allocator){
    return allocator->getOctreeNode(children[i]);
}


bool ChildBlock::isEmpty() {
    for(int i = 0; i < 8; ++i) {
        if(children[i] != UINT_MAX) {
            return false;
        }
    }
    return true;
}

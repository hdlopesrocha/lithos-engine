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

void ChildBlock::clear(OctreeAllocator &allocator, OctreeChangeHandler * handler) {
    for(int i=0; i < 8 ; ++i) {
        uint childNode = children[i];
        if(childNode != UINT_MAX) {
            OctreeNode * child = allocator.get(childNode);
            if(child != NULL) {
                child->clear(allocator, handler, NULL);
                allocator.deallocate(child); // libertar nó
            }
            children[i] = UINT_MAX;
        }
    }
}

void ChildBlock::set(uint i, OctreeNode* node, OctreeAllocator& allocator) {
    uint newIndex = node ? allocator.getIndex(node) : UINT_MAX;
    children[i] = newIndex;
}


OctreeNode * ChildBlock::get(uint i, OctreeAllocator &allocator){
    uint index = children[i];
    if(index == UINT_MAX) return NULL;

    OctreeNode * ptr = allocator.get(index);
    return ptr;
}

bool ChildBlock::isEmpty() {
    for(int i = 0; i < 8; ++i) {
        if(children[i] != UINT_MAX) {
            return false;
        }
    }
    return true;
}

ChildBlock * ChildBlock::deallocate(OctreeAllocator &allocator) {
    if(this->isEmpty()) {
        allocator.childAllocator.deallocate(this);
        return NULL;
    }
    else {
        throw std::runtime_error("ChildBlock::deallocate possible child missing "  );
        return this;
    }
}


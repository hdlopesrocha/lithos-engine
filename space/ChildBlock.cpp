#include "space.hpp"

ChildBlock::ChildBlock() {

}

ChildBlock * ChildBlock::init() {
    memcpy(children, UINT_MAX_ARRAY, sizeof(uint)*8);
    return this;
}

void ChildBlock::clear(OctreeAllocator &allocator, OctreeChangeHandler * handler) {
    OctreeNode * childNodes[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
    allocator.get(childNodes, children);
    for(int i=0; i < 8 ; ++i) {
        OctreeNode * child = childNodes[i];
        if(child != NULL) {
            child->clear(allocator, handler, NULL);
            allocator.deallocate(child); // libertar nó
        }
    }
    memcpy(children, UINT_MAX_ARRAY, sizeof(uint)*8);
}

void ChildBlock::set(uint i, OctreeNode* node, OctreeAllocator& allocator) {
    uint newIndex = node ? allocator.getIndex(node) : UINT_MAX;
    if(newIndex!=UINT_MAX && newIndex >10000000){
        throw std::runtime_error("Ooops!");
    }
    children[i] = newIndex;
}


OctreeNode * ChildBlock::get(uint i, OctreeAllocator &allocator){
    uint index = children[i];
    if(index == UINT_MAX) return NULL;
    if(index!=UINT_MAX && index >10000000){
        throw std::runtime_error("Ooops!");
    }
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



#include "space.hpp"

OctreeNode::OctreeNode(Vertex vertex) {
	init(vertex);
}

OctreeNode * OctreeNode::init(Vertex vertex) {
	this->isSolid = false;
	this->vertex = vertex;
	this->mask = 0x0;
	this->simplified = false;
	this->dataId = 0;
	this->block = NULL;
	return this;
}

void OctreeNode::setChildNode(int i, OctreeNode * node, OctreeAllocator * allocator) {
	if(node==NULL && this->block == NULL) {
		return;
	}
	if(this->block == NULL) {
		this->block = new ChildBlock();
	}
	this->block->children[i] = allocator->nodeAllocator.getIndex(node);
}

OctreeNode * OctreeNode::getChildNode(int i, OctreeAllocator * allocator){
	if(this->block == NULL) {
		return NULL;
	}
	return allocator->nodeAllocator.getFromIndex(this->block->children[i]);
}

OctreeNode::~OctreeNode() {

}

void OctreeNode::clear(OctreeAllocator * allocator) {
	if(this->block != NULL) {
		this->block->clear(allocator);
		delete this->block;
		this->block = NULL;
	}
}

bool OctreeNode::isLeaf() {
	if(this->block == NULL) {
		return true;
	}
    for(int i=0 ; i < 8 ; ++i) {
        if(this->block->children[i] != UINT_MAX) {
            return false;
        }
    }
    return true;
}


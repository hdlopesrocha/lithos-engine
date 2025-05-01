#include "space.hpp"

OctreeNode::OctreeNode(Vertex vertex) {
	init(vertex);
}

OctreeNode * OctreeNode::init(Vertex vertex) {
	this->setSolid(false);
	this->setSimplified(false);
	this->setDirty(false);
	this->vertex = vertex;
	this->mask = 0x0;
	this->dataId = 0;
	this->block = NULL;
	return this;
}

void OctreeNode::setChildNode(int i, OctreeNode * node, OctreeAllocator * allocator) {
	if(node==NULL && this->block == NULL) {
		return;
	}
	if(this->block == NULL) {
		this->block = allocator->childAllocator.allocate()->init();
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
		allocator->childAllocator.deallocate(this->block);
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

bool OctreeNode::isSolid(){
	return this->bits & 0x01;
}

bool OctreeNode::isSimplified(){
	return this->bits & 0x02;
}

bool OctreeNode::isDirty(){
	return this->bits & 0x04;
}

void OctreeNode::setSolid(bool value){
	this->bits = (this->bits & (0x1 ^ 0xff)) | (value ? 0x1 : 0x0);
}

void OctreeNode::setSimplified(bool value){
	this->bits = (this->bits & (0x2 ^ 0xff)) | (value ? 0x2 : 0x0);
}

void OctreeNode::setDirty(bool value){
	this->bits = (this->bits & (0x4 ^ 0xff)) | (value ? 0x4 : 0x0);
}
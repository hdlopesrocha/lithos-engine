#include "space.hpp"

OctreeNode::OctreeNode(Vertex vertex) {
	init(vertex);
}

OctreeNode * OctreeNode::init(Vertex vertex) {
	this->bits = 0x0;
	this->setSolid(false);
	this->setSimplified(false);
	this->setDirty(true);
	this->vertex = vertex;
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
	this->block->children[i] = allocator->getIndex(node);
}

OctreeNode * OctreeNode::getChildNode(int i, OctreeAllocator * allocator){
	if(this->block == NULL) {
		return NULL;
	}
	return allocator->getOctreeNode(this->block->children[i]);
}

OctreeNode::~OctreeNode() {

}

void OctreeNode::clear(OctreeAllocator * allocator, BoundingCube &cube) {
	if(this->block != NULL) {
		this->block->clear(allocator, cube);
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

uint8_t OctreeNode::getMask(){
	return this->bits & 0xff;
}


bool OctreeNode::isSolid(){
	return this->bits & 0x100;
}

bool OctreeNode::isSimplified(){
	return this->bits & 0x200;
}

bool OctreeNode::isDirty(){
	return this->bits & 0x400;
}

void OctreeNode::setSolid(bool value){
	uint16_t mask = 0x0100;
	this->bits = (this->bits & (mask ^ 0xffff)) | (value ? mask : 0x0);
}

void OctreeNode::setSimplified(bool value){
	uint16_t mask = 0x0200;
	this->bits = (this->bits & ~mask) | (value ? mask : 0x0);
}

void OctreeNode::setDirty(bool value){
	uint16_t mask = 0x0400;
	this->bits = (this->bits & ~mask ) | (value ? mask : 0x0);
}

void OctreeNode::setSimplification(uint8_t value){
	uint16_t mask = 0xf000;
	this->bits = (this->bits & (mask ^ 0xffff)) | (value ? mask : 0x0);
}

uint8_t OctreeNode::getSimplification(){
	uint16_t mask = 0xf000;
	return (this->bits & mask >> 12) & 0xf;
}

void OctreeNode::setMask(uint8_t value){
	uint16_t mask = 0x00ff;
	this->bits = (this->bits & ~mask) | (value & mask);
}
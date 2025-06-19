#include "space.hpp"

OctreeNode::OctreeNode(Vertex vertex) {
	init(vertex);
}

OctreeNode * OctreeNode::init(Vertex vertex) {
	for(int i = 0; i < 8; ++i) {
		this->sdf[i] = 0.0f;
	}
	this->bits = 0x0;
	this->setSolid(false);
	this->setSimplified(false);
	this->setDirty(true);
	this->vertex = vertex;
	this->id = UINT_MAX;
	return this;
}

ChildBlock * OctreeNode::getBlock(OctreeAllocator * allocator) {
	return allocator->childAllocator.getFromIndex(this->id);
}

void OctreeNode::setChildNode(int i, OctreeNode * node, OctreeAllocator * allocator) {
	if(node == NULL && this->id == UINT_MAX) {
		return;
	}
	ChildBlock * block = NULL;
	if(this->id == UINT_MAX) {
		block = allocator->childAllocator.allocate()->init();
		this->id = allocator->childAllocator.getIndex(block);
	}
	if(block == NULL) {
		block = allocator->childAllocator.getFromIndex(this->id);
	}
	block->children[i] = allocator->getIndex(node);
}

OctreeNode * OctreeNode::getChildNode(int i, OctreeAllocator * allocator, ChildBlock * block) {
	if(block == NULL) {
		return NULL;
	}
	return allocator->getOctreeNode(block->children[i]);
}

OctreeNode::~OctreeNode() {

}

void OctreeNode::clear(OctreeAllocator * allocator, BoundingCube &cube) {
	if(this->id != UINT_MAX) {
		ChildBlock * block = allocator->childAllocator.getFromIndex(this->id);
		block->clear(allocator, cube);
		allocator->childAllocator.deallocate(block);
		this->id = UINT_MAX;
	}
}

void OctreeNode::setSdf(float * value) {
	for(int i = 0; i < 8; ++i) {
		this->sdf[i] = value[i];
	}
}

bool OctreeNode::isLeaf() {
	return this->id == UINT_MAX;
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


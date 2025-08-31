#include "space.hpp"

OctreeNode::OctreeNode(Vertex vertex) {
	init(vertex);
}

OctreeNode * OctreeNode::init(Vertex vertex) {
	this->bits = 0x0;
	this->sign = 0x0;
	this->setSolid(false);
	this->setEmpty(false);
	this->setSimplified(false);
	this->setDirty(true);
	this->setChunk(false);
	this->setLeaf(false);
	this->vertex = vertex;
	this->id = UINT_MAX;
	return this;
}

ChildBlock * OctreeNode::getBlock(OctreeAllocator * allocator) {
	return allocator->childAllocator.getFromIndex(this->id);
}

ChildBlock * OctreeNode::createBlock(OctreeAllocator * allocator) {
	ChildBlock * block = NULL;
	if(this->id == UINT_MAX) {
		block = allocator->childAllocator.allocate()->init();
		this->id = allocator->childAllocator.getIndex(block);
	}
	if(block == NULL) {
		block = allocator->childAllocator.getFromIndex(this->id);
	}
	return block;
}

void OctreeNode::setChildNode(int i, OctreeNode * node, OctreeAllocator * allocator, ChildBlock * block) {
	if(node == NULL && this->id == UINT_MAX) {
		return;
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

void OctreeNode::clear(OctreeAllocator * allocator, BoundingCube &cube, OctreeChangeHandler * handler) {
	handler->erase(this);
	if(this->id != UINT_MAX) {
		ChildBlock * block = allocator->childAllocator.getFromIndex(this->id);
		block->clear(allocator, cube, handler);
		allocator->childAllocator.deallocate(block);
		this->id = UINT_MAX;
	}
}

void OctreeNode::setSdf(float * value) {
	this->sign = 0x0;
	for(int i = 0; i < 8; ++i) {
		this->sign |= (value[i] < 0.0f ? (0x1 << i) : 0x0);
	}
}

bool OctreeNode::isSolid(){
	return this->bits & (0x1 << 0);
}

void OctreeNode::setSolid(bool value){
	uint8_t mask = (0x1 << 0);
	this->bits = (this->bits & (mask ^ 0xff)) | (value ? mask : 0x0);
}

bool OctreeNode::isEmpty(){
	return this->bits & (0x1 << 1);
}

void OctreeNode::setEmpty(bool value){
	uint8_t mask = (0x1 << 1);
	this->bits = (this->bits & (mask ^ 0xff)) | (value ? mask : 0x0);
}

bool OctreeNode::isSimplified(){
	return this->bits & (0x1 << 2);
}

void OctreeNode::setSimplified(bool value){
	uint8_t mask = (0x1 << 2);
	this->bits = (this->bits & ~mask) | (value ? mask : 0x0);
}

bool OctreeNode::isDirty(){
	return this->bits & (0x1 << 3);
}

void OctreeNode::setDirty(bool value){
	uint8_t mask = (0x1 << 3);
	this->bits = (this->bits & ~mask) | (value ? mask : 0x0);
}

bool OctreeNode::isChunk() {
	return this->bits & (0x1 << 4);
}
void OctreeNode::setChunk(bool value) {
	uint8_t mask = (0x1 << 4);
	this->bits = (this->bits & ~mask) | (value ? mask : 0x0);
}

bool OctreeNode::isLeaf() {
	return this->bits & (0x1 << 5);
}
void OctreeNode::setLeaf(bool value) {
	uint8_t mask = (0x1 << 5);
	this->bits = (this->bits & ~mask) | (value ? mask : 0x0);
}

SpaceType OctreeNode::getType()  {
	if(isSolid()) {
		return SpaceType::Solid;
	} else if(isEmpty()) {
		return SpaceType::Empty;
	} else {
		return SpaceType::Surface;
	}
}

OctreeNode * OctreeNode::compress(OctreeAllocator * allocator, BoundingCube * cube, BoundingCube chunk) {
	int intersectingChildCount = 0;
	int intersectingIndex = -1;

	for (int i = 0; i < 8; ++i) {
		if (cube->getChild(i).intersects(chunk)) {
			intersectingChildCount++;
			intersectingIndex = i;
		}
	}

	if(intersectingChildCount == 1) {
		ChildBlock * block = this->getBlock(allocator);
		if(block != NULL) {
			OctreeNode * childNode = this->getChildNode(intersectingIndex, allocator, block);
			if(childNode != NULL) {
				BoundingCube c = cube->getChild(intersectingIndex);
				*cube = c;
				return childNode->compress(allocator, cube, chunk);
			}
		}
	}
	return this;
}


uint OctreeNode::exportSerialization(OctreeAllocator * allocator, std::vector<OctreeNodeCubeSerialized> * nodes, int * leafNodes, BoundingCube cube, BoundingCube chunk, uint level) {
	if( this->isEmpty() || this->isSolid() || !chunk.intersects(cube)) {
		return 0; // Skip this node
	}
	uint index = nodes->size(); 

	OctreeNodeCubeSerialized n(this->sign, cube, this->vertex, this->bits, level);
	nodes->push_back(n);
	if(isLeaf()) {
		++(*leafNodes);
	}

	ChildBlock * block = this->getBlock(allocator);
	if(block != NULL) {
		for(int i=0; i < 8; ++i) {
			OctreeNode * childNode = this->getChildNode(i, allocator,block);
			if(childNode != NULL) {
				BoundingCube c = cube.getChild(i);
			    (*nodes)[index].children[i] = childNode->exportSerialization(allocator, nodes, leafNodes, c, chunk, level + 1);
			}
		}
	}
	return index;
}


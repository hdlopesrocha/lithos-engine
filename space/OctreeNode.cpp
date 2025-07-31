#include "space.hpp"

OctreeNode::OctreeNode(Vertex vertex) {
	init(vertex);
}

OctreeNode * OctreeNode::init(Vertex vertex) {
	for(int i = 0; i < 8; ++i) {
		this->sdf[i] = MAXFLOAT;
	}
	this->bits = 0x0;
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
	for(int i = 0; i < 8; ++i) {
		this->sdf[i] = value[i];
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


uint OctreeNode::exportSerialization(OctreeAllocator * allocator, std::vector<OctreeNodeCubeSerialized> * nodes, int * leafNodes, BoundingCube cube, BoundingCube chunk, bool isRoot) {

	if( this->isEmpty() || this->isSolid() || !chunk.intersects(cube)) {
		return 0; // Skip this node
	}

	int intersectingChildCount = 0;
	int intersectingIndex = -1;

	if(isRoot) {
		for (int i = 0; i < 8; ++i) {
			if (cube.getChild(i).intersects(chunk)) {
				intersectingChildCount++;
				intersectingIndex = i;
			}
		}
	}
	ChildBlock * block = this->getBlock(allocator);

	uint index = nodes->size(); 

	if(isRoot && intersectingChildCount == 1) {
		if(block != NULL) {
			OctreeNode * childNode = this->getChildNode(intersectingIndex, allocator, block);
			if(childNode != NULL) {
				BoundingCube c = cube.getChild(intersectingIndex);
				return childNode->exportSerialization(allocator, nodes, leafNodes, c, chunk, isRoot);
			}
		}
	}
	else {
		OctreeNodeCubeSerialized n(this->sdf, cube, this->vertex.brushIndex, this->bits);
		nodes->push_back(n);
		if(isLeaf()) {
			++(*leafNodes);
		}


		if(block != NULL) {
			OctreeNodeCubeSerialized * real = &(*nodes)[index];
			for(int i=0; i < 8; ++i) {
				OctreeNode * childNode = this->getChildNode(i, allocator,block);
				if(childNode != NULL) {
					BoundingCube c = cube.getChild(i);
					real->children[i] = childNode->exportSerialization(allocator, nodes, leafNodes, c, chunk, false);
				}
			}
		}
	}
	return index;
}


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
	this->setDirty(false);
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
	// TODO: might be empty if not cleared properly
	return this->id == UINT_MAX;
}


bool OctreeNode::isSolid(){
	return this->bits & 0x01;
}

void OctreeNode::setSolid(bool value){
	uint8_t mask = 0x01;
	this->bits = (this->bits & (mask ^ 0xff)) | (value ? mask : 0x0);
}

bool OctreeNode::isEmpty(){
	return this->bits & 0x02;
}

void OctreeNode::setEmpty(bool value){
	uint8_t mask = 0x02;
	this->bits = (this->bits & (mask ^ 0xff)) | (value ? mask : 0x0);
}

bool OctreeNode::isSimplified(){
	return this->bits & 0x04;
}

void OctreeNode::setSimplified(bool value){
	uint8_t mask = 0x04;
	this->bits = (this->bits & ~mask) | (value ? mask : 0x0);
}

bool OctreeNode::isDirty(){
	return this->bits & 0x08;
}

void OctreeNode::setDirty(bool value){
	uint8_t mask = 0x08;
	this->bits = (this->bits & ~mask) | (value ? mask : 0x0);
}

uint OctreeNode::exportSerialization(OctreeAllocator * allocator, std::vector<OctreeNodeSerialized> * nodes, BoundingCube cube) {
		OctreeNodeSerialized n;
		n.brushIndex = this->vertex.brushIndex;
		n.bits = this->bits;
		SDF::copySDF(this->sdf, n.sdf);

		for(int i=0 ; i < 8 ; ++i) {
			n.children[i] = UINT_MAX;
		}

		uint index = nodes->size(); 
		nodes->push_back(n);

		ChildBlock * block = this->getBlock(allocator);
		if(block != NULL) {
			OctreeNodeSerialized * real = &(*nodes)[index];
			for(int i=0; i < 8; ++i) {
				OctreeNode * childNode = this->getChildNode(i, allocator,block);
				if(childNode != NULL) {
					
					BoundingCube c = cube.getChild(i);
					real->children[i] = childNode->exportSerialization(allocator, nodes, c);
				}
			}
		}
		return index;
}
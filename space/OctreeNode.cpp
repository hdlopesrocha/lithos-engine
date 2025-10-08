#include "space.hpp"

OctreeNode::OctreeNode() {
	init(glm::vec3());
}

OctreeNode::OctreeNode(Vertex vertex) {
	init(vertex);
}

OctreeNode * OctreeNode::init(Vertex vertex) {
	for(int i = 0; i < 8; ++i) {
		this->sdf[i] = INFINITY;
	}
	this->bits = 0x0;
	this->setSolid(false);
	this->setLeaf(false);
	this->setEmpty(false);
	this->setSimplified(false);
	this->setDirty(true);
	this->setChunk(false);
	this->vertex = vertex;
	this->id = UINT_MAX;
	return this;
}

ChildBlock * OctreeNode::getBlock(OctreeAllocator &allocator) {
	return allocator.childAllocator.getFromIndex(this->id);
}

ChildBlock * OctreeNode::allocate(OctreeAllocator &allocator) {
	ChildBlock * block = NULL;
	if(this->id == UINT_MAX) {
		block = allocator.childAllocator.allocate();
		this->id = allocator.childAllocator.getIndex(block);
	}
	if(block == NULL) {
		block = allocator.childAllocator.getFromIndex(this->id);
	}
	return block;
}

ChildBlock * OctreeNode::deallocate(OctreeAllocator &allocator, ChildBlock * block) {
    if(this->id != UINT_MAX && block->isEmpty()) {
        allocator.childAllocator.deallocate(block);
		block = NULL;
		this->id = UINT_MAX;
    }
    else {
        throw std::runtime_error("ChildBlock::deallocate possible child missing "  );
    }
    return block;
}


OctreeNode::~OctreeNode() {

}

ChildBlock * OctreeNode::clear(OctreeAllocator &allocator, OctreeChangeHandler * handler, ChildBlock * block) {
	handler->erase(this);
	if(this->id != UINT_MAX) {
		if(block == NULL) {
			block = getBlock(allocator);
		}
		bool isEmpty = (block!=NULL && block->isEmpty()) || block == NULL;

		if(isEmpty && block!=NULL) {	
			block->clear(allocator, handler);
			block = deallocate(allocator, block);
		} else if(block != NULL) {
			//throw std::runtime_error("OctreeNode::clear possible child missing "  );
		}
	}

	return block;
}

void OctreeNode::setSDF(float * value) {
	for(int i = 0; i < 8; ++i) {
		this->sdf[i] = value != NULL ? value[i] : INFINITY;
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

OctreeNode * OctreeNode::compress(OctreeAllocator &allocator, BoundingCube * cube, BoundingCube chunk) {
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
			OctreeNode * childNode = block->get(intersectingIndex, allocator);
			if(childNode != NULL) {
				BoundingCube c = cube->getChild(intersectingIndex);
				*cube = c;
				return childNode->compress(allocator, cube, chunk);
			}
		}
	}
	return this;
}


uint OctreeNode::exportSerialization(OctreeAllocator &allocator, std::vector<OctreeNodeCubeSerialized> * nodes, int * leafNodes, BoundingCube cube, BoundingCube chunk, uint level) {
	if( this->isEmpty() || this->isSolid() || !chunk.intersects(cube)) {
		return 0; // Skip this node
	}
	uint index = nodes->size(); 

	OctreeNodeCubeSerialized n(this->sdf, cube, this->vertex, this->bits, level);
	nodes->push_back(n);
	if(isLeaf()) {
		++(*leafNodes);
	}

	ChildBlock * block = this->getBlock(allocator);
	if(block != NULL) {
		for(int i=0; i < 8; ++i) {
			OctreeNode * childNode = block->get(i, allocator);
			if(childNode != NULL) {
				BoundingCube c = cube.getChild(i);
			    (*nodes)[index].children[i] = childNode->exportSerialization(allocator, nodes, leafNodes, c, chunk, level + 1);
			}
		}
	}
	return index;
}


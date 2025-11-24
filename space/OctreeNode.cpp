#include "space.hpp"


OctreeNode::OctreeNode() {
	init(glm::vec3());
}

OctreeNode::OctreeNode(Vertex vertex) {
	init(vertex);
}

OctreeNode * OctreeNode::init(Vertex vertex) {
	memcpy(this->sdf, INFINITY_ARRAY, sizeof(float)*8);
	this->bits = 0x0;
	this->setLeaf(false);
	this->setSimplified(false);
	this->setDirty(true);
	this->setChunk(false);
	this->setType(SpaceType::Empty);
	this->vertex = vertex;
	this->id = UINT_MAX;
	return this;
}

ChildBlock * OctreeNode::getBlock(OctreeAllocator &allocator) const {
	return allocator.childAllocator.getFromIndex(this->id);
}


void OctreeNode::setChildren(OctreeAllocator &allocator, uint children[8]) {
	uint blockId = this->id;
	ChildBlock * block = NULL;
	if(blockId == UINT_MAX) {
		block = allocator.childAllocator.allocate()->init();
		this->id = allocator.childAllocator.getIndex(block);
	} else {
		block = allocator.childAllocator.getFromIndex(blockId);
	}
	memcpy(block->children, children, sizeof(uint)*8);
}

void OctreeNode::getChildren(OctreeAllocator &allocator, OctreeNode * childNodes[8]) const {
	ChildBlock * block = getBlock(allocator);
	if(block != NULL) {
	    allocator.get(childNodes, block->children);
	}
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

void OctreeNode::setSDF(float value[8]) {
	memcpy(this->sdf, value, sizeof(float)*8);
}

void OctreeNode::setType(SpaceType type) {
	uint8_t mask = (0x1 << 0) | (0x1 << 1);
	uint8_t value = (type  == SpaceType::Solid ? 0x1 : 0x0) | (type  == SpaceType::Empty ? 0x1 : 0x0) << 1;
	this->bits = (this->bits & (mask ^ 0xff)) | value;
}

bool OctreeNode::isSimplified() const {
	return this->bits & (0x1 << 2);
}

void OctreeNode::setSimplified(bool value){
	uint8_t mask = (0x1 << 2);
	this->bits = (this->bits & ~mask) | (value ? mask : 0x0);
}

bool OctreeNode::isDirty() const {
	return this->bits & (0x1 << 3);
}

void OctreeNode::setDirty(bool value){
	uint8_t mask = (0x1 << 3);
	this->bits = (this->bits & ~mask) | (value ? mask : 0x0);
}

bool OctreeNode::isChunk() const {
	return this->bits & (0x1 << 4);
}
void OctreeNode::setChunk(bool value) {
	uint8_t mask = (0x1 << 4);
	this->bits = (this->bits & ~mask) | (value ? mask : 0x0);
}

bool OctreeNode::isLeaf() const {
	return this->bits & (0x1 << 5);
}

void OctreeNode::setLeaf(bool value) {
	uint8_t mask = (0x1 << 5);
	this->bits = (this->bits & ~mask) | (value ? mask : 0x0);
}

SpaceType OctreeNode::getType() const {
	if(this->bits & (0x1 << 0)) {
		return SpaceType::Solid;
	} else if(this->bits & (0x1 << 1)) {
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
	if( this->getType() != SpaceType::Surface || !chunk.intersects(cube)) {
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


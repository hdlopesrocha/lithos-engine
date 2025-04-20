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
	for(int i=0; i < 8 ; ++i) {
		this->children[i] = UINT_MAX;
	}
	return this;
}

void OctreeNode::setChildNode(int i, OctreeNode * node, Allocator<OctreeNode> * allocator) {
	this->children[i] = allocator->getIndex(node);
}

OctreeNode * OctreeNode::getChildNode(int i, Allocator<OctreeNode> * allocator){
	return allocator->getFromIndex(children[i]);
}

OctreeNode::~OctreeNode() {

}

void OctreeNode::clear(Allocator<OctreeNode> * allocator) {
	for(int i=0; i < 8 ; ++i) {
		OctreeNode * child = allocator->getFromIndex(children[i]);
		if(child != NULL) {
			allocator->deallocate(child);
			setChildNode(i, NULL, allocator);
		}
	}
}

bool OctreeNode::isLeaf() {
    for(int i=0 ; i < 8 ; ++i) {
        if(children[i] != UINT_MAX) {
            return false;
        }
    }
    return true;
}


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
		setChildNode(i, NULL);
	}
	return this;
}

void OctreeNode::setChildNode(int i, OctreeNode * node) {
	this->children[i] = node;
}

OctreeNode * OctreeNode::getChildNode(int i){
	return children[i];
}

OctreeNode::~OctreeNode() {

}

void OctreeNode::clear(Allocator<OctreeNode> * allocator) {
	for(int i=0; i < 8 ; ++i) {
		OctreeNode * child = children[i];
		if(child != NULL) {
			allocator->deallocate(child);
			children[i] = NULL;
		}
	}
}

bool OctreeNode::isLeaf() {
    for(int i=0 ; i < 8 ; ++i) {
        if(children[i] != NULL) {
            return false;
        }
    }
    return true;
}


#include "space.hpp"

OctreeNode::OctreeNode(Vertex vertex, bool leaf) {
	this->solid = ContainmentType::Disjoint;
	this->vertex = vertex;
	this->mask = 0x0;
	this->leaf = leaf;
	this->simplified = false;
	this->dataId = 0;
	for(int i=0; i < 8 ; ++i) {
		setChild(i, NULL);
	}
}

void OctreeNode::setChild(int i, OctreeNode * node) {
	this->children[i] = node;
}

OctreeNode::~OctreeNode() {
	this->clear();
}

void OctreeNode::clear() {
	for(int i=0; i < 8 ; ++i) {
		OctreeNode * child = children[i];
		if(child != NULL) {
			delete child;
			children[i] = NULL;
		}
	}
}


bool OctreeNode::isLeaf() {
	return leaf;
}


bool OctreeNode::isEmpty() {
	for(int i=0; i < 8 ; ++i){
		OctreeNode * child = children[i];
		if(child != NULL) {
			return false;
		}
	}
	return true;
}

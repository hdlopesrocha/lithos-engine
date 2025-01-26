#include "math.hpp"

OctreeNode::OctreeNode(Vertex vertex) {
	this->info = NULL;
	this->infoType = 0;
	this->solid = ContainmentType::Disjoint;
	this->vertex = vertex;
	this->mask = 0x0;
	this->simplified = false;
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

void OctreeNode::clear(){
	for(int i=0; i < 8 ; ++i){
		OctreeNode * child = children[i];
		if(child != NULL) {
			delete child;
			children[i] = NULL;
		}
	}
}

bool OctreeNode::isEmpty(){
	for(int i=0; i < 8 ; ++i){
		OctreeNode * child = children[i];
		if(child != NULL) {
			return false;
		}
	}
	return true;
}



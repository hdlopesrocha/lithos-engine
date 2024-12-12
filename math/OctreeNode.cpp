#include "math.hpp"

OctreeNode::OctreeNode(Vertex vertex) {
	this->vertex = vertex;
    this->solid = ContainmentType::Disjoint;
	this->height = 0;
	for(int i=0; i < 8 ; ++i) {
		setChild(i, NULL);
	}
}

void OctreeNode::setChild(int i, OctreeNode * node) {
	this->children[i] = node;
}

OctreeNode::~OctreeNode() {
	this->solid = ContainmentType::Disjoint;;
	this->clear();
	this->height = 0;
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




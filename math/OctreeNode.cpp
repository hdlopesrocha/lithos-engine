#include "math.hpp"

OctreeNode::OctreeNode(Vertex vertex) {
	this->info = NULL;
	this->infoType = 0;
	this->vertex = vertex;
	this->mask = 0x0;
	for(int i=0; i < 8 ; ++i) {
		setChild(i, NULL);
	}
}

void OctreeNode::setChild(int i, OctreeNode * node) {
	this->children[i] = node;
}

OctreeNode::~OctreeNode() {
	this->mask = 0x0;
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




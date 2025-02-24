#include "math.hpp"

OctreeNode::OctreeNode(Vertex vertex) {
	this->solid = ContainmentType::Disjoint;
	this->vertex = vertex;
	this->mask = 0x0;
	this->simplification = 0;
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

bool OctreeNode::isEmpty() {
	for(int i=0; i < 8 ; ++i){
		OctreeNode * child = children[i];
		if(child != NULL) {
			return false;
		}
	}
	return true;
}


NodeInfo * OctreeNode::getNodeInfo(int infoType) {
	for(int i=0; i < info.size(); ++i){
		NodeInfo * info = &info[i];
		if(info->type == infoType) {
			return info;
		}
	}
	return NULL;
}
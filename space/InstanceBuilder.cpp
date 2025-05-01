#include "space.hpp"


InstanceBuilder::InstanceBuilder(InstanceBuilderHandler * handler, InstanceGeometry * geometry) {
	this->geometry = geometry;
	this->handler = handler;
}

void InstanceBuilder::before(OctreeNodeData &params) {		
	if(params.node->isLeaf()){
		handler->handle(params, geometry);
	}
}

void InstanceBuilder::after(OctreeNodeData &params) {			
	return;
}

bool InstanceBuilder::test(OctreeNodeData &params) {	
	return true;
}

void InstanceBuilder::getOrder(OctreeNodeData &params, uint8_t * order){
	for(size_t i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}

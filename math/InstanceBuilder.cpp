#include <bitset>
#include "math.hpp"


InstanceBuilder::InstanceBuilder(Octree * tree, std::vector<InstanceData> * instances, InstanceBuilderHandler * handler, InstanceGeometry * geometry) {
	this->tree = tree;
	this->instanceCount = 0;
	this->geometry = geometry;
	this->instances = instances;
	this->handler = handler;
}

void InstanceBuilder::before(OctreeNodeData &params) {		
	if(params.height==0){
		handler->handle(params, geometry);
	}
}

void InstanceBuilder::after(OctreeNodeData &params) {			
	return;
}

bool InstanceBuilder::test(OctreeNodeData &params) {	
	return params.height >= 0;
}

void InstanceBuilder::getOrder(OctreeNodeData &params, int * order){
	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}

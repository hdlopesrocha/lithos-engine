#include <bitset>
#include "math.hpp"


InstanceBuilder::InstanceBuilder(Octree * tree, std::vector<InstanceData> * instances, InstanceBuilderHandler * handler) {
	this->tree = tree;
	this->instanceCount = 0;
	this->instances = instances;
	this->handler = handler;
}

void InstanceBuilder::before(IteratorData &params) {		
	if(params.height==0){
		InstanceGeometry * pre= (InstanceGeometry *) params.context;
		handler->handle(params.node, params.cube, params.level, pre);
	}
}

void InstanceBuilder::after(IteratorData &params) {			
	return;
}

bool InstanceBuilder::test(IteratorData &params) {	
	return params.height >= 0;
}

void InstanceBuilder::getOrder(IteratorData &params, int * order){
	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}

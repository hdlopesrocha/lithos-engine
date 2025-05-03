#include "space.hpp"


InstanceBuilder::InstanceBuilder(InstanceBuilderHandler * handler, std::vector<InstanceData> * instances) {
	this->instances = instances;
	this->handler = handler;
}

void InstanceBuilder::before(OctreeNodeData &params) {		
	handler->handle(params, instances);
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

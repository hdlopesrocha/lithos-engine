#include <bitset>
#include "math.hpp"


InstanceBuilder::InstanceBuilder(Octree * tree, std::vector<InstanceData> * instances, InstanceBuilderHandler * handler) {
	this->tree = tree;
	this->instanceCount = 0;
	this->instances = instances;
	this->handler = handler;
}




void * InstanceBuilder::before(int level, int height, int lod, OctreeNode * node, const BoundingCube &cube, void * context) {		

	if(height==0){
		InstanceGeometry * pre= (InstanceGeometry *) context;
		handler->handle(node, cube, level, pre);
	}
	return context; 			 			
}

void InstanceBuilder::after(int level, int height, int lod, OctreeNode * node, const BoundingCube &cube, void * context) {			
	return;
}

bool InstanceBuilder::test(int level, int height, int lod, OctreeNode * node, const BoundingCube &cube, void * context) {	
	return height >= 0;
}


void InstanceBuilder::getOrder(const BoundingCube &cube, int * order){
	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}


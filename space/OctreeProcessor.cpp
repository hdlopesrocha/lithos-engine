#include "space.hpp"


OctreeProcessor::OctreeProcessor(Octree * tree) {
	this->tree = tree;
}

void OctreeProcessor::before(OctreeNodeData &params) {		
	bool canGenerate  = params.node->dataId == 0;
	if(params.cube.getLengthX() <= params.chunkSize){
		if(canGenerate) {
			params.node->dataId = ++tree->dataId;
		}
		params.context = params.node;
	}
}

void OctreeProcessor::after(OctreeNodeData &params) {			
	return;
}

bool OctreeProcessor::test(OctreeNodeData &params) {	
	return loadCount > 0 && params.cube.getLengthX() >= params.chunkSize;
}

void OctreeProcessor::getOrder(OctreeNodeData &params, int * order){
	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}



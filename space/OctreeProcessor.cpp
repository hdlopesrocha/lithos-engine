#include "space.hpp"


OctreeProcessor::OctreeProcessor(Octree * tree) {
	this->tree = tree;
}

bool OctreeProcessor::process(OctreeNodeData &params) {		
	if(params.cube.getLengthX() <= params.chunkSize && params.node->dataId == 0){
		params.node->dataId = ++tree->dataId;
		return true;
	}
	return false;
}

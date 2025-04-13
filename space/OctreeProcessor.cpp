#include "space.hpp"


OctreeProcessor::OctreeProcessor(Octree * tree,bool createInstances, GeometryBuilder * builder) {
	this->tree = tree;
	this->builder = builder;
	this->createInstances = createInstances;
}



void OctreeProcessor::before(OctreeNodeData &params) {		

	NodeInfo * info = params.node->getNodeInfo(builder->infoType);
	bool canGenerate  = info == NULL || info->dirty;

	if(params.cube.getLengthX() <= params.chunkSize){
		if(canGenerate && createInstances) {
			if(info == NULL) {
				params.node->info.push_back(builder->build(params));
				--loadCount;
			}else if(info->dirty) {
				if(info->data != NULL) {
					delete info->data;
				}
				if(info->temp != NULL) {
					delete info->temp;
				}

				*info = builder->build(params);
				info->dirty = false;
			}
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



#include "math.hpp"


OctreeProcessor::OctreeProcessor(Octree * tree,bool createInstances, GeometryBuilder * builder) {
	this->tree = tree;
	this->builder = builder;
	this->createInstances = createInstances;
}



void OctreeProcessor::before(IteratorData &params) {		

	NodeInfo * info = params.node->getNodeInfo(builder->infoType);
	bool canGenerate  = info == NULL || info->dirty;

	if(params.lod==0){
		if(canGenerate && createInstances) {
			if(info == NULL) {
				params.node->info.push_back(builder->build(params.level, params.height, params.lod, params.node, params.cube));
				--loadCount;
			}else {
				//TODO replace info, delete vs new ?
				info->dirty = false;
			}
		}
		params.context = params.node;
	}
}

void OctreeProcessor::after(IteratorData &params) {			
	return;
}

bool OctreeProcessor::test(IteratorData &params) {	
	return loadCount > 0 && params.lod>=0;
}

void OctreeProcessor::getOrder(IteratorData &params, int * order){
	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}



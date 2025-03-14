#include <bitset>
#include "../math/math.hpp"
#include "gl.hpp"




OctreeProcessor::OctreeProcessor(Octree * tree,bool createInstances, GeometryBuilder * builder) {
	this->tree = tree;
	this->builder = builder;
	this->createInstances = createInstances;
}



void * OctreeProcessor::before(int level, int height, int lod , OctreeNode * node, const BoundingCube &cube, void * context) {		

	NodeInfo * info = node->getNodeInfo(builder->drawableType);
	bool canGenerate  = info == NULL || info->dirty;

	if(lod==0){
		if(canGenerate && createInstances) {
			if(info == NULL) {
				node->info.push_back(builder->build(level, height, lod, node, cube));
				--loadCount;
			}else {
				//TODO replace info, delete vs new ?
				info->dirty = false;
			}
		}
		return node;
	}
	
	return NULL; 			 			
}

void OctreeProcessor::after(int level, int height, int lod, OctreeNode * node, const BoundingCube &cube, void * context) {			
	return;
}

bool OctreeProcessor::test(int level, int height, int lod, OctreeNode * node, const BoundingCube &cube, void * context) {	
	return loadCount > 0 && lod>=0;
}

void OctreeProcessor::getOrder(const BoundingCube &cube, int * order){
	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}



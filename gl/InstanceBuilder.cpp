#include <bitset>
#include "../math/math.hpp"
#include "gl.hpp"

InstanceBuilder::InstanceBuilder(Octree * tree, std::vector<InstanceData> * instances) {
	this->tree = tree;
	this->instanceCount = 0;
	this->instances = instances;
}

void * InstanceBuilder::before(int level, int height, int lod, OctreeNode * node, const BoundingCube &cube, void * context) {		
	OctreeNode * neighbors[8];

	if(height==0){
		
		Vertex * v = &node->vertex;
		if(v->brushIndex == 2) { 
			//neighbors[0] = node;
			tree->getNodeNeighbors(cube, level, 0, 1, neighbors, 0, 8);

			OctreeNodeTriangleInstanceBuilder handler(&chunk, &instanceCount , (OctreeNode**)&neighbors, instances, 3);
			tree->handleQuadNodes(*node, neighbors, handler);

		}
	}
	return NULL; 			 			
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


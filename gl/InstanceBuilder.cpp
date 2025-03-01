#include <bitset>
#include "../math/math.hpp"
#include "gl.hpp"
#include <glm/gtx/norm.hpp> 

InstanceBuilder::InstanceBuilder(Octree * tree, int lod, std::vector<InstanceData> * instances) {
	this->tree = tree;
	this->instanceCount = 0;
	this->lod = lod;
	this->instances = instances;
}

void * InstanceBuilder::before(int level, OctreeNode * node, BoundingCube * cube, void * context) {		
	int height = tree->getHeight(cube);
	OctreeNode * neighbors[8];

	if(height==lod){
		
		Vertex * v = &node->vertex;
		if(v->brushIndex == 2) { 
			//neighbors[0] = node;
			tree->getNodeNeighbors(cube, level, 0, 1, neighbors, 0, 8);

			OctreeNodeTriangleInstanceBuilder handler(&chunk, &instanceCount , (OctreeNode**)&neighbors, instances, 3);
			tree->handleQuadNodes(node, neighbors, &handler);

		}
		
		return node;
	}
	return NULL; 			 			
}

void InstanceBuilder::after(int level, OctreeNode * node, BoundingCube * cube, void * context) {			
	return;
}

bool InstanceBuilder::test(int level, OctreeNode * node, BoundingCube * cube, void * context) {	
	return context == NULL;
}


void InstanceBuilder::getOrder(BoundingCube * cube, int * order){
	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}


#include <bitset>
#include "../math/math.hpp"
#include "gl.hpp"
#include <glm/gtx/norm.hpp> 

InstanceBuilder::InstanceBuilder(Octree * tree) {
	this->tree = tree;
	this->instanceCount = 0;
}


OctreeNode * InstanceBuilder::getChild(OctreeNode * node, int index){
	return node->children[index];
}

void * InstanceBuilder::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
	if(tree->getHeight(cube)==0){
		
		Vertex * v = &node->vertex;
		if(v->brushIndex == 2) { 
			OctreeNode * corners[8];

			OctreeNode ** cornersPointer = (OctreeNode **) corners;
			tree->getNodeCorners(cube, level, 0, 1, corners);

			QuadNodeInstanceBuilderHandler handler(&chunk, &instanceCount ,cornersPointer, &instances);
			tree->handleQuadNodes(node, corners, &handler);

		}
		
		return node;
	}
	return NULL; 			 			
}

void InstanceBuilder::after(int level, OctreeNode * node, BoundingCube cube, void * context) {			
	return;
}

bool InstanceBuilder::test(int level, OctreeNode * node, BoundingCube cube, void * context) {	
	return context == NULL;
}


void InstanceBuilder::getOrder(OctreeNode * node, BoundingCube cube, int * order){
	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}


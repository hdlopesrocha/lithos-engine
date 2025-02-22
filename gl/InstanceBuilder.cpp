#include <bitset>
#include "../math/math.hpp"
#include "gl.hpp"
#include <glm/gtx/norm.hpp> 

InstanceBuilder::InstanceBuilder(Octree * tree,  int drawableType, int geometryLevel, int * triangles) {
	this->tree = tree;
	this->drawableType = drawableType;
	this->geometryLevel = geometryLevel;
	this->triangles = triangles;
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

			QuadNodeInstanceBuilderHandler handler(&chunk, triangles,cornersPointer, &matrices);
			tree->getQuadNodes(corners, &handler, triangles);

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


/*
void OctreeRenderer::getOrder(OctreeNode * node, BoundingCube cube, int * order){
	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}
*/
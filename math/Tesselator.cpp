#include <bitset>
#include "math.hpp"
#include <bits/stdc++.h>

//      6-----7
//     /|    /|
//    4z+---5 |
//    | 2y--+-3
//    |/    |/
//    0-----1x



Tesselator::Tesselator(Octree * tree, Geometry * chunk, int simplification) {
	this->tree = tree;
	this->simplification = simplification;
	this->chunk = chunk;
	this->triangles = 0;

}

void * Tesselator::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
	OctreeNode * corners[8];

	if(tree->getHeight(cube)==0){
		tree->getNodeNeighbors(cube, level, simplification, 1, corners, 0, 8);
		
		// Tesselate
		OctreeNodeTriangleTesselator handler(chunk, &triangles);
		tree->handleQuadNodes(node, corners , &handler);	
	}
	return context;
}

void Tesselator::after(int level, OctreeNode * node, BoundingCube cube, void * context) {
	return;
}

bool Tesselator::test(int level, OctreeNode * node, BoundingCube cube, void * context) {			
	return node->solid != ContainmentType::Contains && tree->getHeight(cube) >= 0;
}

void Tesselator::getOrder(OctreeNode * node, BoundingCube cube, int * order){
	for(int i = 7 ; i >= 0 ; --i) {
		order[i] = i;
	}
}



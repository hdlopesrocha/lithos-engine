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

void * Tesselator::before(int level, int height, OctreeNode * node, BoundingCube &cube, void * context) {		
	static OctreeNode * corners[7];

	if(height==0){
		tree->getNodeNeighbors(cube, level, simplification, 1, corners, 0, 8);
		
		// Tesselate
		OctreeNodeTriangleTesselator handler(chunk, &triangles);
		tree->handleQuadNodes(node, corners , &handler);	
	}
	return context;
}

void Tesselator::after(int level, int height, OctreeNode * node, BoundingCube &cube, void * context) {
	return;
}

bool Tesselator::test(int level, int height, OctreeNode * node, BoundingCube &cube, void * context) {			
	return node->solid != ContainmentType::Contains && tree->getHeight(cube) >= 0;
}

void Tesselator::getOrder(BoundingCube &cube, int * order){
	for(int i = 7 ; i >= 0 ; --i) {
		order[i] = i;
	}
}



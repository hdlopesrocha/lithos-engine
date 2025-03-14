#include <bitset>
#include "math.hpp"
#include <bits/stdc++.h>

Tesselator::Tesselator(Octree * tree, Geometry * chunk, int simplification) {
	this->tree = tree;
	this->simplification = simplification;
	this->chunk = chunk;
	this->triangles = 0;

}

void * Tesselator::before(int level, int height, int lod, OctreeNode * node, const BoundingCube &cube, void * context) {		

	if(height==0){		
		// Tesselate
		OctreeNodeTriangleTesselator handler(chunk, &triangles);
		tree->handleQuadNodes(cube, level,*node , &handler);	
	}
	return context;
}

void Tesselator::after(int level, int height, int lod, OctreeNode * node, const BoundingCube &cube, void * context) {
	return;
}

bool Tesselator::test(int level, int height, int lod, OctreeNode * node, const BoundingCube &cube, void * context) {			
	return node->solid != ContainmentType::Contains && height >= 0;
}

void Tesselator::getOrder(const BoundingCube &cube, int * order){
	for(int i = 7 ; i >= 0 ; --i) {
		order[i] = i;
	}
}



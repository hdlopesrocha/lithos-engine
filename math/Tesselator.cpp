#include <bitset>
#include "math.hpp"
#include <bits/stdc++.h>

Tesselator::Tesselator(Octree * tree, Geometry * chunk, int simplification) {
	this->tree = tree;
	this->simplification = simplification;
	this->chunk = chunk;
	this->triangles = 0;

}

void Tesselator::before(IteratorData &params) {		

	if(params.height==0){		
		// Tesselate
		OctreeNodeTriangleTesselator handler(chunk, &triangles);
		tree->handleQuadNodes(params.cube, params.level,*params.node , &handler);	
	}
}

void Tesselator::after(IteratorData &params) {
	return;
}

bool Tesselator::test(IteratorData &params) {			
	return params.node->solid != ContainmentType::Contains && params.height >= 0;
}

void Tesselator::getOrder(IteratorData &params, int * order){
	for(int i = 7 ; i >= 0 ; --i) {
		order[i] = i;
	}
}



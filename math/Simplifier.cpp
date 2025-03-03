#include <bitset>
#include "math.hpp"
#include <bits/stdc++.h>

Simplifier::Simplifier(Octree * tree, BoundingCube chunkCube, float angle, float distance, bool texturing, int simplification) {
	this->tree = tree;
	this->chunkCube = chunkCube;
	this->simplification = simplification;
	this->angle = angle;
	this->distance = distance;
	this->texturing = texturing;
	this->chunkCube = chunkCube;
}

void * Simplifier::before(int level, int height, OctreeNode * node, const BoundingCube &cube, void * context) {		
	return context; 			 			
}

void Simplifier::after(int level, int height, OctreeNode * node, const BoundingCube &cube, void * context) {
	// The parentNode plane

	Plane parentPlane(node->vertex.normal, node->vertex.position); 
	const Vertex parentVertex = node->vertex;
	
	if(height == 0) {
		node->simplification = simplification;
		return;
	}

	for(int i=0; i < 8 ; ++i) {
		BoundingCube cc(cube.getMin() - cube.getLengthX()*Octree::getShift(i), cube.getLengthX());
		OctreeNode * c = tree->getNodeAt(cc.getCenter(), level, 0);
		if(c!=NULL && c->solid == ContainmentType::Intersects) {
			if(!chunkCube.contains(cc)){
				return;
			}

			if(parentVertex.brushIndex != c->vertex.brushIndex && texturing) {
				return;	
			}
			
			float d = parentPlane.distance(c->vertex.position);
			if( d > distance ) {
				return;
			}

			float a = glm::dot(parentVertex.normal, c->vertex.normal);
			if(a < angle){
				return;
			}
		}
	}

	glm::vec3 sumP = glm::vec3(0);
	glm::vec3 sumN = glm::vec3(0);
	int nodeCount=0;

	// for leaf nodes shouldn't loop
	for(int i=0; i < 8 ; ++i) {
		OctreeNode * c = node->children[i];
		if(c!=NULL && c->solid == ContainmentType::Intersects) {
			if(c->simplification != simplification) {
				return;
			}
			if(parentVertex.brushIndex != c->vertex.brushIndex && texturing) {
				return;	
			}
			sumP += c->vertex.position;
			sumN += c->vertex.normal;
			++nodeCount;
		}
	}

	if(nodeCount > 0) {	
		node->vertex.position = sumP / (float)nodeCount;
		node->simplification = simplification;
	}


	return;
}

bool Simplifier::test(int level, int height, OctreeNode * node, const BoundingCube &cube, void * context) {			
	return node->solid != ContainmentType::Contains && height >= 0;
}

void Simplifier::getOrder(const BoundingCube &cube, int * order){
	for(int i = 7 ; i >= 0 ; --i) {
		order[i] = i;
	}
}
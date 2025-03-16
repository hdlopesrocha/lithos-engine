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

void Simplifier::before(IteratorData &params) {		

}

void Simplifier::after(IteratorData &params) {
	// The parentNode plane

	Plane parentPlane(params.node->vertex.normal, params.node->vertex.position); 
	const Vertex parentVertex = params.node->vertex;
	
	if(params.height == 0) {
		params.node->simplification = simplification;
		return;
	}

	for(int i=0; i < 8 ; ++i) {
		BoundingCube cc(params.cube.getMin() - params.cube.getLengthX()*Octree::getShift(i), params.cube.getLengthX());
		OctreeNode * c = tree->getNodeAt(cc.getCenter(), params.level, 0);
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
		OctreeNode * c = params.node->children[i];
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
		params.node->vertex.position = sumP / (float)nodeCount;
		params.node->simplification = simplification;
	}


	return;
}

bool Simplifier::test(IteratorData &params) {			
	return params.node->solid != ContainmentType::Contains && params.height >= 0;
}

void Simplifier::getOrder(IteratorData &params, int * order){
	for(int i = 7 ; i >= 0 ; --i) {
		order[i] = i;
	}
}
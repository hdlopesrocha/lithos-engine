#include <bitset>
#include "math.hpp"
#include <bits/stdc++.h>

Simplifier::Simplifier(Octree * tree, float angle, float distance, bool texturing) {
	this->tree = tree;
	this->angle = angle;
	this->distance = distance;
	this->texturing = texturing;
}

void simplify(Octree * tree, OctreeNode * node, BoundingCube cube, BoundingCube * chunkCube, int level, float distance, float angle, bool texturing) {
	BoundingCube outerCube(cube.getMin() -cube.getLength(), cube.getLength());
	if(chunkCube == NULL || !chunkCube->contains(outerCube)) {
		return;
	}

	bool canSimplify = true;
	uint mask = node->mask;

	glm::vec3 sumP = glm::vec3(0);
	glm::vec3 sumN = glm::vec3(0);
	int nodeCount=0;


	// The parentNode plane
	Plane parentPlane(node->vertex.normal, node->vertex.position); 
	Vertex parentVertex = node->vertex;
	

	//std::vector<OctreeNode*> nodes = tree->getNeighbors(cube, level);

	BoundingCube childCube = Octree::getChildCube(cube, 0);
	//std::vector<OctreeNode*> nodes = tree->getNodeCorners(childCube, level+1, false, 1);
	std::vector<OctreeNode*> nodes;
	for (int i=0 ; i < 8 ; ++i) {
		nodes.push_back(node->children[i]);
	}

	// for leaf nodes shouldn't loop
	for(int i=0; i <nodes.size() ; ++i) {
		OctreeNode * c = nodes[i];
		if(c!=NULL && c->solid == ContainmentType::Intersects) {
		    float d = parentPlane.distance(c->vertex.position);
			float a = glm::dot(parentVertex.normal, c->vertex.normal);
			if(!c->simplified || a < angle || d > distance || (parentVertex.texIndex != c->vertex.texIndex && texturing)){
				canSimplify = false;
				break;
			}
			sumP += c->vertex.position;
			sumN += c->vertex.normal;
			++nodeCount;
		}
	}

	if(canSimplify && nodeCount > 0)  {
		//node->clear();
		parentVertex.position = sumP / (float)nodeCount;
		node->vertex = parentVertex;
		//vertex.normal = sumN / (float)nodeCount;
		//node->mask = mask;
	} 

	node->simplified = canSimplify;
}

OctreeNode * Simplifier::getChild(OctreeNode * node, int index){
	OctreeNode * child = node->children[index];
	return child != NULL ? child : NULL;
}

void * Simplifier::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
	return context; 			 			
}

void Simplifier::after(int level, OctreeNode * node, BoundingCube cube, void * context) {
	int height = tree->getHeight(cube);
	BoundingCube * chunkCube = (BoundingCube*) context;
	if(chunkCube != NULL) {
		simplify(tree, node, cube, chunkCube, level, distance, angle, texturing);	
	}
	return;
}

bool Simplifier::test(int level, OctreeNode * node, BoundingCube cube, void * context) {			
	return node->solid != ContainmentType::Contains && tree->getHeight(cube) >= 0;
}

std::vector<int> Simplifier::getOrder(OctreeNode * node){
	std::vector<int> nodes;
	for(int i = 7 ; i >= 0 ; --i) {
		nodes.push_back(i);
	}
	return nodes;
}
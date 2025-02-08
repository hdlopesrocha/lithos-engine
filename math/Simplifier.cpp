#include <bitset>
#include "math.hpp"
#include <bits/stdc++.h>

Simplifier::Simplifier(Octree * tree, float angle, float distance, bool texturing, int simplification) {
	this->tree = tree;
	this->simplification = simplification;
	this->angle = angle;
	this->distance = distance;
	this->texturing = texturing;
}

bool isBorder(BoundingCube chunk, BoundingCube c) {
	return chunk.getMinX() == c.getMinX() || chunk.getMinY() == c.getMinY() || chunk.getMinZ() == c.getMinZ();
}

void Simplifier::simplify(Octree * tree, OctreeNode * node, BoundingCube cube, BoundingCube * chunkCube, int level) {
	bool canSimplify = true;
	uint mask = node->mask;

	glm::vec3 sumP = glm::vec3(0);
	glm::vec3 sumN = glm::vec3(0);
	int nodeCount=0;

	// The parentNode plane
	Plane parentPlane(node->vertex.normal, node->vertex.position); 
	Vertex parentVertex = node->vertex;
	
	//std::vector<OctreeNode*> nodes = tree->getNodeCorners(childCube, level+1, false, 1);
	//std::vector<OctreeNode*> nodes = tree->getNeighbors(cube, level);
	OctreeNode* nodes[8];
	for (int i=0 ; i < 8 ; ++i) {
		nodes[i] =node->children[i];
	}

	// for leaf nodes shouldn't loop
	for(int i=0; i < 8 ; ++i) {
		OctreeNode * c = nodes[i];
		if(c!=NULL && c->solid == ContainmentType::Intersects) {
			if(parentVertex.texIndex != c->vertex.texIndex && texturing) {
				canSimplify = false;
				break;		
			}
			
			BoundingCube childCube = Octree::getChildCube(cube, i);
			if(isBorder(*chunkCube, childCube)){
				canSimplify = false;
				break;
			}
		
		    float d = parentPlane.distance(c->vertex.position);
			if( d > distance ) {
				canSimplify = false;
				break;
			}

			float a = glm::dot(parentVertex.normal, c->vertex.normal);
			if(a < angle){
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
		node->simplification = simplification;
	} 

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
		simplify(tree, node, cube, chunkCube, level);	
	}
	return;
}

bool Simplifier::test(int level, OctreeNode * node, BoundingCube cube, void * context) {			
	return node->solid != ContainmentType::Contains && tree->getHeight(cube) >= 0;
}

void Simplifier::getOrder(OctreeNode * node, BoundingCube cube, int * order){
	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}
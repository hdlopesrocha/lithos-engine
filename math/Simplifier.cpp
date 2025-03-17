#include <bitset>
#include "math.hpp"

Simplifier::Simplifier(Octree * tree, BoundingCube chunkCube, float angle, float distance, bool texturing) {
	this->tree = tree;
	this->chunkCube = chunkCube;
	this->angle = angle;
	this->distance = distance;
	this->texturing = texturing;
}	

void Simplifier::before(OctreeNodeData &params) {		
	
}

void Simplifier::after(OctreeNodeData &params) {

	if(params.height == 0) {
		params.node->simplified = true;
		return;
	}else {
		params.node->simplified = false;
	}

	bool hasSimplifiedChildren = false;
	for(int i=0; i < 8 ; ++i) {
		OctreeNode * c = params.node->children[i];
		if(c != NULL && c->simplified) {
			hasSimplifiedChildren = true;
			break;
		}
	}
	if(hasSimplifiedChildren) {
		// The parentNode plane
		const Vertex parentVertex = params.node->vertex;
		Plane parentPlane(params.node->vertex.normal, parentVertex.position); 

		for(int i=0; i < 8 ; ++i) {
			BoundingCube cc(params.cube.getMin() - params.cube.getLengthX()*Octree::getShift(i), params.cube.getLengthX());
			OctreeNode * c = tree->getNodeAt(cc.getCenter(), params.level, true);

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
				if(a < angle) {
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
				if(!c->simplified) {
					return;	
				}
				sumP += c->vertex.position;
				sumN += c->vertex.normal;
				++nodeCount;
			}
		}
		

		if(nodeCount > 0) {	
			params.node->simplified = true;
			params.node->vertex.position = sumP / (float)nodeCount;
		}
	}
	return;
}

bool Simplifier::test(OctreeNodeData &params) {			
	return params.node->solid != ContainmentType::Contains && params.height >= 0;
}

void Simplifier::getOrder(OctreeNodeData &params, int * order){
	for(int i = 7 ; i >= 0 ; --i) {
		order[i] = i;
	}
}
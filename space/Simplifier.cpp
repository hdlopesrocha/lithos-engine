#include "space.hpp"


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

	if(params.node->isLeaf()) {
		params.node->simplified = true;
		return;
	}else {
		params.node->simplified = false;
	}

	bool hasSimplifiedChildren = false;
	for(int i=0; i < 8 ; ++i) {
		OctreeNode * c = params.node->getChildNode(i);
		if(c != NULL && c->simplified) {
			hasSimplifiedChildren = true;
			break;
		}
	}
	if(hasSimplifiedChildren) {
		// The parentNode plane
		OctreeNode * parent = params.node;
		Vertex * parentVertex = &parent->vertex;
		Plane parentPlane(parent->vertex.normal, parentVertex->position); 

		BoundingCube cube(params.cube.getMin() - params.cube.getLengthX(), params.cube.getLengthX());
		if(!chunkCube.contains(cube)){
			return;
		}
		

		glm::vec3 sumP = glm::vec3(0);
		glm::vec3 sumN = glm::vec3(0);
		uint mask = 0xff;
		int nodeCount=0;

		// for leaf nodes shouldn't loop
		for(int i=0; i < 8 ; ++i) {
			OctreeNode * node = params.node->getChildNode(i);
			if(node!=NULL && !node->isSolid) {
				if(!node->simplified) {
					return;	
				}
				if(texturing && node->vertex.brushIndex != parentVertex->brushIndex) {
					return;	
				}
				
				float d = parentPlane.distance(node->vertex.position);
				if( d > distance*cube.getLengthX() ) {
					return;
				}

				float a = glm::dot(parentVertex->normal, node->vertex.normal);
				if(a < angle) {
					return;
				}
				
				if(mask != 0xff && mask != node->mask) {
					return;
				}
				
				mask = node->mask;
				sumP += node->vertex.position;
				sumN += node->vertex.normal;
				
				++nodeCount;
			}
		}
		

		if(nodeCount > 0) {	
			parent->simplified = true;
			parentVertex->position = sumP / (float)nodeCount;
			parentVertex->normal = sumN / (float)nodeCount;
			parent->mask = mask;
			parent->isSolid = false;
		}
	}
	return;
}

bool Simplifier::test(OctreeNodeData &params) {			
	return !params.node->isSolid;
}

void Simplifier::getOrder(OctreeNodeData &params, int * order){
	for(int i = 7 ; i >= 0 ; --i) {
		order[i] = i;
	}
}
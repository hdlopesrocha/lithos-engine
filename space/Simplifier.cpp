#include "space.hpp"


Simplifier::Simplifier(float angle, float distance, bool texturing) {
	this->angle = angle;
	this->distance = distance;
	this->texturing = texturing;
}	


void Simplifier::simplify(BoundingCube chunkCube, const OctreeNodeData &params){

	if(params.node->isLeaf()) {
		params.node->setSimplified(true);
		return;
	}else {
		params.node->setSimplified(false);
	}

	ChildBlock * block = params.node->getBlock(params.allocator);

	bool hasSimplifiedChildren = false;
	for(int i=0; i < 8 ; ++i) {
		OctreeNode * c = params.node->getChildNode(i, params.allocator, block);
		if(c != NULL && c->isSimplified()) {
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
		//uint mask = 0xff;
		int nodeCount=0;


		// for leaf nodes shouldn't loop
		for(int i=0; i < 8 ; ++i) {
			OctreeNode * node = params.node->getChildNode(i, params.allocator, block);
			if(node!=NULL && !node->isSolid()) {
				if(!node->isSimplified()) {
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
				/*
				if(mask != 0xff && mask != node->getMask()) {
					return;
				}
				
				mask = node->getMask();
				*/
				sumP += node->vertex.position;
				sumN += node->vertex.normal;
				
				++nodeCount;
			}
		}
		

		if(nodeCount > 0) {	
			parent->setSimplified(true);
			parentVertex->position = sumP / (float)nodeCount;
			parentVertex->normal = sumN / (float)nodeCount;
			//parent->setMask(mask);
			parent->setSolid(false);
		}
	}
	return;
}

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
	OctreeNode * node = params.node;
	ChildBlock * block = node->getBlock(params.allocator);

	bool hasSimplifiedChildren = false;
	for(int i=0; i < 8 ; ++i) {
		OctreeNode * c = node->getChildNode(i, params.allocator, block);
		if(c != NULL && c->isSimplified()) {
			hasSimplifiedChildren = true;
			break;
		}
	}
	if(hasSimplifiedChildren) {
		// The parentNode plane
		Vertex * nodeVertex = &node->vertex;
		Plane parentPlane(nodeVertex->normal, nodeVertex->position); 

		BoundingCube cube(params.cube.getMin() - params.cube.getLength(), params.cube.getLengthX());
		if(!chunkCube.contains(cube)){
			return;
		}

		//uint mask = 0xff;
		int nodeCount=0;

		// for leaf nodes shouldn't loop
		for(int i=0; i < 8 ; ++i) {
			OctreeNode * child = node->getChildNode(i, params.allocator, block);
			if(child!=NULL && !child->isSolid() && !child->isEmpty()) {
				if(!child->isSimplified()) {
					return;	
				}
				if(texturing && child->vertex.brushIndex != nodeVertex->brushIndex) {
					return;	
				}
				
				glm::vec3 p0 = child->vertex.position;
				float d = parentPlane.distance(p0);
				if( d > distance*cube.getLengthX() ) {
					return;
				}

				float a = glm::dot(nodeVertex->normal, child->vertex.normal);
				if(a < angle) {
					return;
				}
				
				++nodeCount;
			}
		}
		

		if(nodeCount > 0) {	
			//std::cout << "Simplifying node " << node->id << " with " << nodeCount << " children." << std::endl;
			node->setSimplified(true);
		}
	}
	return;
}

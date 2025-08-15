#include "space.hpp"


Simplifier::Simplifier(float maxError, bool texturing) {
	this->maxError = maxError;
	this->texturing = texturing;
}	


void Simplifier::simplify(BoundingCube chunkCube, const OctreeNodeData &params){
	if(params.node->isLeaf()) {
		params.node->setSimplified(true);
		return;
	} else {
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
			BoundingCube childCube = params.cube.getChild(i);


			if(child!=NULL && !child->isSolid() && !child->isEmpty()) {
				if(!child->isSimplified()) {
					return;	
				}

				for(int j = 0 ; j < 8 ; ++j) {
					glm::vec3 corner = childCube.getCorner(j);
					float d = SDF::interpolate(params.node->sdf, corner , params.cube);
					float dif = glm::abs(d - child->sdf[j]);

					if(dif > params.cube.getLengthX() * maxError) {
						return;
					}
				}

				if(texturing && child->vertex.brushIndex != nodeVertex->brushIndex) {
					return;	
				}
				
				++nodeCount;
			}
		}

		if(nodeCount > 0) {	
			std::cout << "Simplifying node " << node->id << " with " << nodeCount << " children." << std::endl;
			node->setSimplified(true);
			for(int i=0; i < 8 ; ++i) {
				OctreeNode * child = node->getChildNode(i, params.allocator, block);
				if(child!=NULL) {
					child->setSimplified(false);
				}
			}
		}
	}
	return;
}

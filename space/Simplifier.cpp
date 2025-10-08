#include "space.hpp"


Simplifier::Simplifier(float angle, float distance, bool texturing) {
	this->angle = angle;
	this->distance = distance;
	this->texturing = texturing;
}	


void Simplifier::simplify(Octree * tree, BoundingCube chunkCube, const OctreeNodeData &params, ChildBlock * block){
	OctreeNode * node = params.node;
	int brushIndex = 0;
	bool hasSimplifiedChildren = false;
	if(block!=NULL) {
		for(int i=0; i < 8 ; ++i) {
			OctreeNode * c = block->get(i, *tree->allocator);
			if(c != NULL && c->isSimplified()) {
				hasSimplifiedChildren = true;
				brushIndex = c->vertex.brushIndex;
				break;
			}
		}
	}
	if(hasSimplifiedChildren) {
		// The parentNode plane
		Vertex * nodeVertex = &node->vertex;
		Plane parentPlane(nodeVertex->normal, nodeVertex->position); 

		BoundingCube cube(params.cube.getMin() - params.cube.getLength(), params.cube.getLengthX());
		if(!chunkCube.contains(cube)) {
			return;
		}

		//uint mask = 0xff;
		int nodeCount=0;

		// for leaf nodes shouldn't loop
		for(int i=0; i < 8 ; ++i) {
			OctreeNode * child = block->get(i, *tree->allocator);
			if(child!=NULL && !child->isSolid() && !child->isEmpty()) {
				if(!child->isSimplified()) {
					return;	
				}
				if(texturing && child->vertex.brushIndex != brushIndex) {
					return;	
				}
				BoundingCube childCube = params.cube.getChild(i);


				
				for(int j = 0 ; j < 8 ; ++j) {
					glm::vec3 corner = childCube.getCorner(j);
					float d = SDF::interpolate(params.node->sdf, corner , params.cube);
					float dif = glm::abs(d - child->sdf[j]);

					if(dif > params.cube.getLengthX() * 0.05) {
						return;
					}
				}



				++nodeCount;
			}
		}
		

		if(nodeCount > 0) {	
			//std::cout << "Simplifying node " << node->id << " with " << nodeCount << " children." << std::endl;
			node->setSimplified(true);
			node->vertex.brushIndex = brushIndex;

			for(int i=0; i < 8 ; ++i) {
				OctreeNode * child = block->get(i, *tree->allocator);
				if(child!=NULL) {
					child->setSimplified(false);
				}
			}
		}
	}
	return;
}

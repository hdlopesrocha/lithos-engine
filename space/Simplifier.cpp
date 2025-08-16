#include "space.hpp"


Simplifier::Simplifier(float maxError, bool texturing) {
	this->maxError = maxError;
	this->texturing = texturing;
}	


void Simplifier::simplify(BoundingCube chunkCube, const OctreeNodeData &params){
	OctreeNode * node = params.node;

	if(node->isLeaf() || node->isSolid() || node->isEmpty()) {
		node->setSimplified(true);
		return;
	} else {
		node->setSimplified(false);
	}
	ChildBlock * block = node->getBlock(params.allocator);
	OctreeNode * children[8] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL };
	OctreeNode * anyChild = NULL;

	bool hasSimplifiedChildren = false;

	for(int i=0; i < 8 ; ++i) {
		OctreeNode * child = node->getChildNode(i, params.allocator, block);

		if(child != NULL && child->isSimplified() && !child->isSolid() && !child->isEmpty()) {
			hasSimplifiedChildren = true;
			anyChild = child;
		}
		children[i] = child;
	}

	if(hasSimplifiedChildren) {
		// The parentNode plane
		BoundingCube cube(params.cube.getMin() - params.cube.getLength(), params.cube.getLengthX());
		if(!chunkCube.contains(cube)){
			return;
		}

		int nodeCount=0;

		// for leaf nodes shouldn't loop
		for(int i=0; i < 8 ; ++i) {
			OctreeNode * child = children[i];
			if(child!=NULL) {
				BoundingCube childCube = params.cube.getChild(i);
				if(child->isEmpty() || child->isSolid()) {
					continue;
				}

				for(int j = 0 ; j < 8 ; ++j) {
					glm::vec3 corner = childCube.getCorner(j);
					float d = SDF::interpolate(params.node->sdf, corner , params.cube);
					float dif = glm::abs(d - child->sdf[j]);

					if(dif > params.cube.getLengthX() * maxError) {
						return;
					}
				}

				if(texturing && child->vertex.brushIndex != anyChild->vertex.brushIndex) {
					return;	
				}
				
				++nodeCount;
			}
		}

		if(nodeCount > 0) {
			//std::cout << "Simplifying node " << node->id << " with " << nodeCount << " children. Level = " << params.level << std::endl;
			node->setSimplified(true);
			node->vertex.brushIndex = anyChild->vertex.brushIndex;
			for(int i=0; i < 8 ; ++i) {
				OctreeNode * child = children[i];
				if(child!=NULL) {
					child->setSimplified(false);
				}
			}
		}
	}
	return;
}

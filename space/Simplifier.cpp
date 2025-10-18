#include "space.hpp"


Simplifier::Simplifier(float angle, float distance, bool texturing) {
	this->angle = angle;
	this->distance = distance;
	this->texturing = texturing;
}	


bool Simplifier::simplify(const BoundingCube chunkCube, const BoundingCube cube, const float * sdf, NodeOperationResult * children){	
	int brushIndex = 0;
	bool hasSimplifiedChildren = false;
	for(int i=0; i < 8 ; ++i) {
		NodeOperationResult * child = &children[i];
		if(child->isSimplified) {
			hasSimplifiedChildren = true;
			brushIndex = child->brushIndex;
			break;
		}
	}

	if(hasSimplifiedChildren) {
		if(!chunkCube.contains(BoundingCube(cube.getMin() - cube.getLength(), cube.getLengthX()))) {
			return false;
		}

		//uint mask = 0xff;
		int nodeCount=0;

		// for leaf nodes shouldn't loop
		for(int i=0; i < 8 ; ++i) {
			NodeOperationResult * child = &children[i];
			if(child->resultType == SpaceType::Surface) {
				if(!child->isSimplified) {
					return false;	
				}
				if(texturing && child->brushIndex != brushIndex) {
					return false;	
				}
				BoundingCube childCube = cube.getChild(i);

				for(int j = 0 ; j < 8 ; ++j) {
					glm::vec3 corner = childCube.getCorner(j);
					float d = SDF::interpolate(sdf, corner , cube);
					float dif = glm::abs(d - child->resultSDF[j]);

					if(dif > cube.getLengthX() * 0.05) {
						return false;
					}
				}

				++nodeCount;
			}
		}
		
		if(nodeCount > 0) {	
			return true;
		}
	}
	return false;
}

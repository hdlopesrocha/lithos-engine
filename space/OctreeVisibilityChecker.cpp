#include "space.hpp"


OctreeVisibilityChecker::OctreeVisibilityChecker(std::vector<OctreeNodeData> * visibleNodes) {
	this->visibleNodes = visibleNodes;
}

void OctreeVisibilityChecker::update(glm::mat4 m) {
	frustum = Frustum(m);
}

void OctreeVisibilityChecker::before(OctreeNodeData &params) {		
	if(params.cube.getLengthX() <= params.chunkSize){
		visibleNodes->push_back({params.level, params.chunkSize, params.node, params.cube, params.context, params.allocator});
	}
}

void OctreeVisibilityChecker::after(OctreeNodeData &params) {			
	return;
}

bool OctreeVisibilityChecker::test(OctreeNodeData &params) {
	if(params.cube.getLengthX() >= params.chunkSize) {
		if(params.context == NULL) {
			ContainmentType containmentType = frustum.test(params.cube);
			if(containmentType == ContainmentType::Contains) {
				params.context = params.node;
				return true;
			}
			else if(containmentType == ContainmentType::Intersects) {
				return true;
			}
		}else {
			return true;
		}
	}

	return false;
}


void OctreeVisibilityChecker::getOrder(OctreeNodeData &params, uint8_t * order){
	static std::pair<float, int> internalSortingVector[8]={};
	
	for(int i =0; i< 8; ++i){
		BoundingCube c = params.cube.getChild(i);
		internalSortingVector[i] = std::pair<float, int>(glm::distance2(c.getCenter(), sortPosition), i);
	}

    std::sort(std::begin(internalSortingVector), std::end(internalSortingVector), 
		[&](const std::pair<float, int>& a, const std::pair<float, int>& b) {
        return a.first < b.first;
    });

	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = internalSortingVector[i].second;
	}
}

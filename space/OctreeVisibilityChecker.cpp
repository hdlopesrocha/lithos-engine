#include "space.hpp"


OctreeVisibilityChecker::OctreeVisibilityChecker() {
	visibleNodes.clear();
}

void OctreeVisibilityChecker::update(glm::mat4 m) {
	frustum = Frustum(m);
}

void OctreeVisibilityChecker::before(Octree * tree, OctreeNodeData *params) {		
	
}

void OctreeVisibilityChecker::after(Octree * tree, OctreeNodeData *params) {			
	if(params->context != NULL) {
		params->context = NULL;
		visibleNodes.push_back(*params);
	}
}

bool OctreeVisibilityChecker::test(Octree * tree, OctreeNodeData *params) {
	if(params != NULL && params->context == NULL) {	
		ContainmentType containmentType = frustum.test(params->cube);
		if(containmentType == ContainmentType::Disjoint) {
			return false;
		}
		if(params->node != NULL) {
			if(params->node->isChunk()) {
				params->context = params->node;
			}
			return true;
		}
	}
	return false;
}


void OctreeVisibilityChecker::getOrder(Octree * tree, OctreeNodeData *params, uint8_t * order){
	static std::pair<float, int> internalSortingVector[8]={};
	
	for(int i =0; i< 8; ++i){
		BoundingCube c = params->cube.getChild(i);
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

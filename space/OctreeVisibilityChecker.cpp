#include "space.hpp"


OctreeVisibilityChecker::OctreeVisibilityChecker() {
	visibleNodes.reserve(1024);
}

void OctreeVisibilityChecker::update(glm::mat4 m) {
	frustum = Frustum(m);
	viewDir = glm::normalize(-glm::vec3(m[2]));
}

void OctreeVisibilityChecker::before(const Octree &tree, OctreeNodeData &params) {		
	
}

void OctreeVisibilityChecker::after(const Octree &tree, OctreeNodeData &params) {			
	if(params.context != NULL) {
		params.context = NULL;
		std::lock_guard<std::mutex> lock(mutex);
		visibleNodes.push_back(params);
	}
}

bool OctreeVisibilityChecker::test(const Octree &tree, OctreeNodeData &params) {
	if(params.context == NULL) {	
		ContainmentType containmentType = params.containmentType == ContainmentType::Contains ? params.containmentType : frustum.test(params.cube);
		if(containmentType == ContainmentType::Disjoint) {
			return false;
		}

		if(params.node->isChunk()) {
			if(params.node->getType() == SpaceType::Surface) {
				params.context = params.node;
			} else {
				return false;
			}
		}
		params.containmentType = containmentType;
		return true;
	}
	return false;
}

void OctreeVisibilityChecker::getOrder(const Octree &tree, OctreeNodeData &params, uint8_t order[8]){
    std::pair<float, uint> internalSortingVector[8]={};

    for(uint i = 0; i < 8; ++i){
        glm::vec3 center = params.cube.getChildCenter(i);
	    float key = -glm::dot(center, viewDir);
        internalSortingVector[i] = std::pair<float, uint>(key, i);
    }

    std::sort(std::begin(internalSortingVector), std::end(internalSortingVector),
        [&](const std::pair<float, uint>& a, const std::pair<float, uint>& b) {
            return a.first < b.first;
    });

    for(uint i = 0 ; i < 8 ; ++i) {
        order[i] = internalSortingVector[i].second;
    }
}
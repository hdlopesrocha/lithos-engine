#include "space.hpp"


OctreeVisibilityChecker::OctreeVisibilityChecker(std::vector<OctreeNodeData> * visibleNodes) {
	this->visibleNodes = visibleNodes;
}

void OctreeVisibilityChecker::update(glm::mat4 m) {
	frustum = Frustum(m);
}

void OctreeVisibilityChecker::before(Octree * tree, OctreeNodeData &params) {		
	if(params.node->isChunk()){
		visibleNodes->push_back({params.level, params.node, params.cube, params.context});
	}
}

void OctreeVisibilityChecker::after(Octree * tree, OctreeNodeData &params) {			
	return;
}

bool OctreeVisibilityChecker::test(Octree * tree, OctreeNodeData &params) {
	if(params.context != NULL) {
		return false;
	}

	ContainmentType containmentType = frustum.test(params.cube);
	if(containmentType == ContainmentType::Disjoint) {
		return false;
	}

	if(params.node->isChunk()) {
		params.context = params.node;
	}

	return true;
}


void OctreeVisibilityChecker::getOrder(Octree * tree, OctreeNodeData &params, uint8_t * order){
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

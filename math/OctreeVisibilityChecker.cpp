#include <bitset>
#include "math.hpp"

OctreeVisibilityChecker::OctreeVisibilityChecker(Octree * tree, std::vector<IteratorData> * visibleNodes) {
	this->tree = tree;
	this->visibleNodes = visibleNodes;
}

void OctreeVisibilityChecker::update(glm::mat4 m) {
	frustum = Frustum(m);
}



void OctreeVisibilityChecker::before(IteratorData &params) {		
	if(params.lod == 0){
		visibleNodes->push_back({params.level, params.height, params.lod, params.node, params.cube, params.context});
	}
}

void OctreeVisibilityChecker::after(IteratorData &params) {			
	return;
}

bool OctreeVisibilityChecker::test(IteratorData &params) {
	if(params.lod >=0) {
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


void OctreeVisibilityChecker::getOrder(IteratorData &params, int * order){
	static std::pair<float, int> internalSortingVector[8]={};
	
	for(int i =0; i< 8; ++i){
		BoundingCube c = Octree::getChildCube(params.cube, i);
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

#include <bitset>
#include "math.hpp"

OctreeVisibilityChecker::OctreeVisibilityChecker(Octree * tree, std::vector<IteratorData> * visibleNodes) {
	this->tree = tree;
	this->visibleNodes = visibleNodes;
}

void OctreeVisibilityChecker::update(glm::mat4 m) {
	frustum = Frustum(m);
}



void * OctreeVisibilityChecker::before(int level, int height, int lod, OctreeNode * node, const BoundingCube &cube, void * context) {		
	if(lod == 0){
		visibleNodes->push_back({level, height, lod, node, cube, context});
	}
	return NULL;
}

void OctreeVisibilityChecker::after(int level, int height, int lod, OctreeNode * node, const BoundingCube &cube, void * context) {			
	return;
}

bool OctreeVisibilityChecker::test(int level, int height, int lod, OctreeNode * node, const BoundingCube &cube, void * context) {	
	return lod >= 0 && frustum.isBoxVisible(cube);
}


void OctreeVisibilityChecker::getOrder(const BoundingCube &cube, int * order){
	static std::pair<float, int> internalSortingVector[8]={};
	
	for(int i =0; i< 8; ++i){
		BoundingCube c = Octree::getChildCube(cube, i);
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

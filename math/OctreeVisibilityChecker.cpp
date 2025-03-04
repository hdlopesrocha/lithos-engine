#include <bitset>
#include "math.hpp"

OctreeVisibilityChecker::OctreeVisibilityChecker(Octree * tree, int geometryLevel, std::vector<IteratorData> * visibleNodes) {
	this->tree = tree;
	this->visibleNodes = visibleNodes;
	this->geometryLevel = geometryLevel;
}

void OctreeVisibilityChecker::update(glm::mat4 m) {
	frustum = Frustum(m);
}



void * OctreeVisibilityChecker::before(int level, int height, OctreeNode * node, const BoundingCube &cube, void * context) {		
	int currentLod = height - geometryLevel;

	if(currentLod == 0){
		visibleNodes->push_back({level, height, node, cube, context});
		return node;
	}
	return NULL;
}

void OctreeVisibilityChecker::after(int level, int height, OctreeNode * node, const BoundingCube &cube, void * context) {			
	return;
}

bool OctreeVisibilityChecker::test(int level, int height, OctreeNode * node, const BoundingCube &cube, void * context) {	
	if(context != NULL) {
		return false;
	}
	BoundingBox box = BoundingBox(cube.getMin()-tree->minSize, cube.getMax());
	return frustum.isBoxVisible(box);
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

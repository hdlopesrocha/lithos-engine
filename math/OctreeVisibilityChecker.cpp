#include <bitset>
#include "math.hpp"
#include <glm/gtx/norm.hpp> 

OctreeVisibilityChecker::OctreeVisibilityChecker(Octree * tree, int geometryLevel, std::vector<IteratorData> * visibleNodes) {
	this->tree = tree;
	this->visibleNodes = visibleNodes;
	this->geometryLevel = geometryLevel;
}

void OctreeVisibilityChecker::update(glm::mat4 m) {
	frustum = Frustum(m);
}



void * OctreeVisibilityChecker::before(int level, OctreeNode * node, BoundingCube &cube, void * context) {		
	float height = tree->getHeight(cube);
	int currentLod = height - geometryLevel;

	if(currentLod <= 0){
		visibleNodes->push_back({level, node, cube, context});
		return node;
	}
	return NULL;
}

void OctreeVisibilityChecker::after(int level, OctreeNode * node, BoundingCube &cube, void * context) {			
	return;
}

bool OctreeVisibilityChecker::test(int level, OctreeNode * node, BoundingCube &cube, void * context) {	
	if(context != NULL) {
		return false;
	}
	BoundingBox box = BoundingBox(cube.getMin()-tree->minSize, cube.getMax());
	return frustum.isBoxVisible(box);
}


void OctreeVisibilityChecker::getOrder(BoundingCube &cube, int * order){
	static std::pair<glm::vec3, int> internalSortingVector[8]={};
	
	for(int i =0; i< 8; ++i){
		BoundingCube c = Octree::getChildCube(cube, i);
		internalSortingVector[i] = std::pair<glm::vec3, int>(c.getCenter(), i);
	}

    std::sort(std::begin(internalSortingVector), std::end(internalSortingVector), 
		[&](const std::pair<glm::vec3, int>& a, const std::pair<glm::vec3, int>& b) {
	    float distA = glm::distance2(a.first, sortPosition);
        float distB = glm::distance2(b.first, sortPosition);
        return distA < distB;
    });

	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = internalSortingVector[i].second;
	}
}

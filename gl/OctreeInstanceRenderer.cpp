#include <bitset>
#include "../math/math.hpp"
#include "gl.hpp"
#include <glm/gtx/norm.hpp> 

OctreeInstanceRenderer::OctreeInstanceRenderer(GLuint program, Octree * tree, int mode, int drawableType, int geometryLevel) {
	this->tree = tree;
	this->mode = mode;
	this->program = program;
	this->drawableType = drawableType;
	this->geometryLevel = geometryLevel;
	this->instances = 0;
}

void OctreeInstanceRenderer::update(glm::mat4 m) {
	frustum = Frustum(m);
}

OctreeNode * OctreeInstanceRenderer::getChild(OctreeNode * node, int index){
	return node->children[index];
}

void * OctreeInstanceRenderer::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
	for(int i=0; i < node->info.size(); ++i){
		NodeInfo * info = &node->info[i];
		// drawable geometry
		if(info->type == drawableType){
			DrawableInstanceGeometry * drawable = (DrawableInstanceGeometry*) info->data;
			//std::cout << "Draw " << std::to_string(drawable->instancesCount) << " | " << std::to_string(drawableType) << std::endl;

			drawable->draw(mode);
			instances += drawable->instancesCount;
		}
	}
	if(tree->getHeight(cube)==geometryLevel){
		return node;
	}
	return NULL;
}

void OctreeInstanceRenderer::after(int level, OctreeNode * node, BoundingCube cube, void * context) {			
	return;
}

bool OctreeInstanceRenderer::test(int level, OctreeNode * node, BoundingCube cube, void * context) {	
	BoundingBox box = BoundingBox(cube.getMin()-tree->minSize, cube.getMax());
	return frustum.isBoxVisible(box) && context == NULL;
}


void OctreeInstanceRenderer::getOrder(OctreeNode * node, BoundingCube cube, int * order){
	std::pair<glm::vec3, int> internalSortingVector[8]={};
	
	for(int i =0; i< 8; ++i){
		BoundingCube c = Octree::getChildCube(cube, i);
		internalSortingVector[i] = std::pair<glm::vec3, int>(c.getCenter(), i);
	}



    std::sort(std::begin(internalSortingVector), std::end(internalSortingVector), 
		[&](const std::pair<glm::vec3, int>& a, const std::pair<glm::vec3, int>& b) {
	    float distA = glm::distance2(a.first, cameraPosition);
        float distB = glm::distance2(b.first, cameraPosition);
        return distA < distB;
    });

	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = internalSortingVector[i].second;
	}
}

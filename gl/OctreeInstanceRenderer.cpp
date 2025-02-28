#include <bitset>
#include "../math/math.hpp"
#include "gl.hpp"
#include <glm/gtx/norm.hpp> 

OctreeInstanceRenderer::OctreeInstanceRenderer(Octree * tree, int * instances, int mode, int drawableType, int geometryLevel, Settings * settings) {
	this->tree = tree;
	this->mode = mode;
	this->drawableType = drawableType;
	this->geometryLevel = geometryLevel;
	this->instances = instances;
	this->settings = settings;
}

void OctreeInstanceRenderer::update(glm::mat4 m) {
	frustum = Frustum(m);
}

OctreeNode * OctreeInstanceRenderer::getChild(OctreeNode * node, int index){
	return node->children[index];
}

void * OctreeInstanceRenderer::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
	float height = tree->getHeight(cube);
	int currentLod = height - geometryLevel;
	float range = settings->billboardRange;

	float amount = Math::clamp(1.0- glm::distance2(cameraPosition, cube.getCenter())/(range * range), 0.0f, 1.0f); 
	if(amount > 0.8) {
		amount = 1.0;
	}

	if(currentLod <= 0){
		for(int i=0; i < node->info.size(); ++i){
			NodeInfo * info = &node->info[i];
			// drawable geometry
			if(info->type == drawableType){
				DrawableInstanceGeometry * drawable = (DrawableInstanceGeometry*) info->data;
				//std::cout << "Current LOD " << std::to_string(currentLod) << " | " << std::to_string(selectedLod) << std::endl;

				//std::cout << "Draw " << std::to_string(drawable->instancesCount) << " | " << std::to_string(drawableType) << std::endl;
				if(drawableType == TYPE_INSTANCE_VEGETATION_DRAWABLE) {
					drawable->draw(mode, amount);
					*instances += drawable->instancesCount*amount;
				} else {
					drawable->draw(mode);
					*instances += drawable->instancesCount;
				}
			}
		}
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
	    float distA = glm::distance2(a.first, sortPosition);
        float distB = glm::distance2(b.first, sortPosition);
        return distA < distB;
    });

	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = internalSortingVector[i].second;
	}
}

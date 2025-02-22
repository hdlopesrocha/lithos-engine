#include <bitset>
#include "../math/math.hpp"
#include "gl.hpp"
#include <glm/gtx/norm.hpp> 

OctreeRenderer::OctreeRenderer(Octree * tree,  int drawableType, int geometryLevel) {
	this->tree = tree;
	this->drawableType = drawableType;
	this->geometryLevel = geometryLevel;
}

void OctreeRenderer::update(glm::mat4 m) {
	frustum = Frustum(m);
}

OctreeNode * OctreeRenderer::getChild(OctreeNode * node, int index){
	return node->children[index];
}

void * OctreeRenderer::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
	if(tree->getHeight(cube)==geometryLevel){
		for(int i=0; i < node->info.size(); ++i){
			NodeInfo * info = &node->info[i];
			// drawable geometry
			if(info->type == drawableType) {
				DrawableInstanceGeometry * drawable = (DrawableInstanceGeometry*) info->data;
				Geometry * loadable = (Geometry*) info->temp;

				if(loadable != NULL) {
					if(drawable!=NULL) {
						delete drawable;
					}
					std::vector<glm::mat4> instances;
					instances.push_back(glm::mat4(1.0));
					drawable = new DrawableInstanceGeometry(loadable, &instances);
					info->data = drawable;
					info->temp = NULL;
					delete loadable;
				}

				drawable->draw(mode);
			}	
		}	
		return node;
	}
	return NULL; 			 			
}

void OctreeRenderer::after(int level, OctreeNode * node, BoundingCube cube, void * context) {			
	return;
}

bool OctreeRenderer::test(int level, OctreeNode * node, BoundingCube cube, void * context) {	
	BoundingBox box = BoundingBox(cube.getMin()-tree->minSize, cube.getMax());
	return frustum.isBoxVisible(box) && context == NULL;
}


void OctreeRenderer::getOrder(OctreeNode * node, BoundingCube cube, int * order){
	std::pair<glm::vec3, int> internalSortingVector[8];
	
	for(int i =0; i< 8; ++i){
		BoundingCube c = Octree::getChildCube(cube, i);
		internalSortingVector[i] = std::pair<glm::vec3, int>(c.getCenter(), i);
	}

    std::sort(internalSortingVector, internalSortingVector+8, [&]( std::pair<glm::vec3, int>& a, std::pair<glm::vec3, int>& b) {
	    float distA = glm::distance2(a.first, cameraPosition);
        float distB = glm::distance2(b.first, cameraPosition);
        return distA < distB;
    });

	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = internalSortingVector[i].second;
	}
}


/*
void OctreeRenderer::getOrder(OctreeNode * node, BoundingCube cube, int * order){
	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}
*/
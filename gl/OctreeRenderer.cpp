#include <bitset>
#include "../math/math.hpp"
#include "gl.hpp"


OctreeRenderer::OctreeRenderer(Octree * tree, int geometryType,  int drawableType) {
	this->tree = tree;
	this->geometryType = geometryType;
	this->drawableType = drawableType;
}

void OctreeRenderer::update(glm::mat4 m) {
	frustum = Frustum(m);
}


OctreeNode * OctreeRenderer::getChild(OctreeNode * node, int index){
	return node->children[index];
}

void * OctreeRenderer::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		

	for(int i=0; i < node->info.size(); ++i){
		NodeInfo info = node->info[i];

		// just geometry not drawable
		if(info.type == geometryType && loaded == 0) {
			Geometry * i = (Geometry*) info.data;
			info.data = new DrawableGeometry(i);
			info.type = drawableType;
			++loaded;
			delete i;
		}

		// drawable geometry
		if(info.type == drawableType) {
			DrawableGeometry * geo = (DrawableGeometry*) info.data;
			geo->draw(mode);
		}	
		node->info[i] = info;
	}
	return NULL; 			 			
}

void OctreeRenderer::after(int level, OctreeNode * node, BoundingCube cube, void * context) {			
	return;
}

bool OctreeRenderer::test(int level, OctreeNode * node, BoundingCube cube, void * context) {	
	BoundingBox box = BoundingBox(cube.getMin()-tree->minSize, cube.getMax());
	return frustum.isBoxVisible(box);
}


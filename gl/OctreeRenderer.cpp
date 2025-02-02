#include <bitset>
#include "../math/math.hpp"
#include "gl.hpp"


OctreeRenderer::OctreeRenderer(Octree * tree) {
	this->tree = tree;

}

void OctreeRenderer::update(glm::mat4 m) {
	frustum = Frustum(m);
}


OctreeNode * OctreeRenderer::getChild(OctreeNode * node, int index){
	return node->children[index];
}

void * OctreeRenderer::before(int level, OctreeNode * node, BoundingCube cube, void * context) {			
	if(node->info != NULL){
		// just geometry not drawable
		if(node->infoType == 0 && loaded == 0) {
			Geometry * info = (Geometry*) node->info;
			node->info = new DrawableGeometry(info);
			node->infoType = 1;
			++loaded;
			delete info;
		}

		// drawable geometry
		if(node->infoType == 1) {
			DrawableGeometry * info = (DrawableGeometry*) node->info;
			info->draw(mode);
		}	
		
	}
	return NULL; 			 			
}

void OctreeRenderer::after(int level, OctreeNode * node, BoundingCube cube, void * context) {			
	return;
}

bool OctreeRenderer::test(int level, OctreeNode * node, BoundingCube cube, void * context) {	
	// -cube.length because surface nets is creating geometry outside the chunk
	// TODO: prevent surface nets from doing this, currently just a quickfix
	BoundingBox box = BoundingBox(cube.getMin()-cube.getLength(), cube.getMax());
	return frustum.isBoxVisible(box);
}


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
	NodeInfo info = node->nodeInfo;

	if(info.data != NULL){
		// just geometry not drawable
		if(info.type == 0 && loaded == 0) {
			Geometry * i = (Geometry*) info.data;
			info.data = new DrawableGeometry(i);
			info.type = 1;
			node->nodeInfo = info;
			++loaded;
			delete i;
		}

		// drawable geometry
		if(info.type == 1) {
			DrawableGeometry * i = (DrawableGeometry*) info.data;
			i->draw(mode);
		}	
		
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


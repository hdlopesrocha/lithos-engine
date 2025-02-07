#include <bitset>
#include "../math/math.hpp"
#include "gl.hpp"


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
	if(tree->getHeight(cube)==geometryLevel && node->info.size()==0 && loaded == 0){
		Simplifier simplifier(tree); 
		simplifier.iterate(level, node, cube, &cube);
		
		Tesselator tesselator(tree);
		Geometry * chunk = new Geometry();
		tesselator.iterate(level, node, cube, chunk);
		NodeInfo info;
		info.data = new DrawableGeometry(chunk);
		info.type = drawableType;
		node->info.push_back(info);
		delete chunk;
		++loaded;

	}

	for(int i=0; i < node->info.size(); ++i){
		NodeInfo info = node->info[i];

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

std::vector<int> OctreeRenderer::getOrder(OctreeNode * node){
	std::vector<int> nodes;
	for(int i = 0 ; i < 8 ; ++i) {
		nodes.push_back(i);
	}
	return nodes;
}
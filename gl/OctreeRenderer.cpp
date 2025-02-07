#include <bitset>
#include "../math/math.hpp"
#include "gl.hpp"


OctreeRenderer::OctreeRenderer(Octree * tree, int * triangles,  int drawableType, int geometryLevel, float simplificationAngle, float simplificationDistance) {
	this->tree = tree;
	this->simplificationAngle = simplificationAngle;
	this->simplificationDistance = simplificationDistance;
	this->drawableType = drawableType;
	this->geometryLevel = geometryLevel;
	this->triangles = triangles;
}

void OctreeRenderer::update(glm::mat4 m) {
	frustum = Frustum(m);
}


OctreeNode * OctreeRenderer::getChild(OctreeNode * node, int index){
	return node->children[index];
}

void * OctreeRenderer::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
	bool canGenerate = true;
	for(int i=0; i < node->info.size(); ++i){
		NodeInfo info = node->info[i];
		if(info.type == drawableType) {
			canGenerate = false;
		}
	}
	
	if(tree->getHeight(cube)==geometryLevel && canGenerate && loaded == 0){
		// Simplify
		Simplifier simplifier(tree, simplificationAngle, simplificationDistance); 
		simplifier.iterate(level, node, cube, &cube);
		
		// Tesselate
		Geometry * chunk = new Geometry();
		Tesselator tesselator(tree, triangles, chunk);
		tesselator.iterate(level, node, cube, NULL);

		// Send to GPU
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
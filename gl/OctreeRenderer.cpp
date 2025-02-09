#include <bitset>
#include "../math/math.hpp"
#include "gl.hpp"
#include <glm/gtx/norm.hpp> 

OctreeRenderer::OctreeRenderer(Octree * tree, int * triangles,  int drawableType, int geometryLevel, float simplificationAngle, float simplificationDistance, bool simplificationTexturing) {
	this->tree = tree;
	this->simplificationAngle = simplificationAngle;
	this->simplificationDistance = simplificationDistance;
	this->simplificationTexturing = simplificationTexturing;
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
static int simplficationId = 0;
void * OctreeRenderer::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
	bool canGenerate = true;
	for(int i=0; i < node->info.size(); ++i){
		NodeInfo info = node->info[i];
		if(info.type == drawableType) {
			canGenerate = false;
		}
	}
	
	if(tree->getHeight(cube)==geometryLevel){
	
		if(canGenerate && loaded == 0){
			++simplficationId;
			// Simplify
			Simplifier simplifier(tree, simplificationAngle, simplificationDistance, simplificationTexturing, simplficationId); 
			simplifier.iterate(level, node, cube, &cube);
			
			// Tesselate
			Geometry * chunk = new Geometry();
			Tesselator tesselator(tree, triangles, chunk, simplficationId);
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
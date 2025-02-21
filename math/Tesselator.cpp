#include <bitset>
#include "math.hpp"
#include <bits/stdc++.h>

//      6-----7
//     /|    /|
//    4z+---5 |
//    | 2y--+-3
//    |/    |/
//    0-----1x



Tesselator::Tesselator(Octree * tree, int * triangles, Geometry * chunk, int simplification) {
	this->tree = tree;
	this->simplification = simplification;
	this->chunk = chunk;
	this->triangles = triangles;

}

int addQuad(OctreeNode** quad, Geometry * chunk, bool reverse) {
	OctreeNode* c0 = quad[reverse ? 3:0];
	OctreeNode* c1 = quad[reverse ? 2:1];
	OctreeNode* c2 = quad[reverse ? 1:2];
	OctreeNode* c3 = quad[reverse ? 0:3];


	Vertex v0 = c0->vertex;
	Vertex v1 = c1->vertex;
	Vertex v2 = c2->vertex;
	Vertex v3 = c3->vertex;

	float scale = 0.1;
	int plane = Math::triplanarPlane(v0.position, v0.normal);
	v0.texCoord = Math::triplanarMapping(v0.position, plane)*scale;
	v1.texCoord = Math::triplanarMapping(v1.position, plane)*scale;
	v2.texCoord = Math::triplanarMapping(v2.position, plane)*scale;
	v3.texCoord = Math::triplanarMapping(v3.position, plane)*scale;	

	int count = 0;


	if(c0!= c1 && c1 != c2 && c0!=c2 && c0->vertex.brushIndex>=0 && c1->vertex.brushIndex>=0 && c2->vertex.brushIndex>=0){
		chunk->addVertex(v0, true);
		chunk->addVertex(v2, true);
		chunk->addVertex(v1, true);
		++count;
	}

	if(c0!= c3 && c3 != c2 && c0!=c2 && c0->vertex.brushIndex>=0 && c2->vertex.brushIndex>=0 && c3->vertex.brushIndex>=0){
		chunk->addVertex(v0, true);
		chunk->addVertex(v3, true);
		chunk->addVertex(v2, true);
		++count;
	}
	//std::cout << "One" << std::endl;
	return count;
}

OctreeNode * Tesselator::getChild(OctreeNode * node, int index) {
	OctreeNode * child = node->children[index];
	return child != NULL ? child : NULL;
}


QuadNodeHandler::QuadNodeHandler(Geometry * chunk, int * triangles) {
	this->chunk = chunk;
	this->triangles = triangles;
}

void QuadNodeHandler::handle(OctreeNode** quad, bool sign) {
	*triangles += addQuad(quad, chunk, sign);
}

void * Tesselator::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
	if(tree->getHeight(cube)==0){
		OctreeNode * corners[8];
		tree->getNodeCorners(cube, level, simplification, 1, corners);
		// Tesselate
		QuadNodeHandler handler(chunk, triangles);
		tree->getQuadNodes(corners , &handler, triangles);	
	}
	return context;
}

void Tesselator::after(int level, OctreeNode * node, BoundingCube cube, void * context) {
	return;
}

bool Tesselator::test(int level, OctreeNode * node, BoundingCube cube, void * context) {			
	return node->solid != ContainmentType::Contains && tree->getHeight(cube) >= 0;
}

void Tesselator::getOrder(OctreeNode * node, BoundingCube cube, int * order){
	for(int i = 7 ; i >= 0 ; --i) {
		order[i] = i;
	}
}
#include <bitset>
#include "math.hpp"
#include <bits/stdc++.h>

//      6-----7
//     /|    /|
//    4z+---5 |
//    | 2y--+-3
//    |/    |/
//    0-----1x



Tesselator::Tesselator(Octree * tree, Geometry * chunk, int simplification) {
	this->tree = tree;
	this->simplification = simplification;
	this->chunk = chunk;
	this->triangles = 0;

}



OctreeNode * Tesselator::getChild(OctreeNode * node, int index) {
	OctreeNode * child = node->children[index];
	return child != NULL ? child : NULL;
}

void * Tesselator::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
	if(tree->getHeight(cube)==0){
		OctreeNode * corners[8];
		tree->getNodeCorners(cube, level, simplification, 1, corners);
		// Tesselate


		QuadNodeTesselatorHandler handler(chunk, &triangles);
		tree->getQuadNodes(corners , &handler);	
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



int addTriangle(OctreeNode* c0, OctreeNode* c1, OctreeNode* c2, Geometry * chunk, bool reverse) {
    int count = 0;
    if(c0 != NULL && c1 != NULL && c2!=NULL) {
        Vertex v0 = c0->vertex;
        Vertex v1 = c1->vertex;
        Vertex v2 = c2->vertex;

        if(c0!= c1 && c1 != c2 && c0!=c2 && c0->vertex.brushIndex>=0 && c1->vertex.brushIndex>=0 && c2->vertex.brushIndex>=0){
            chunk->addVertex(reverse ? v2 : v0, true);
            chunk->addVertex(reverse ? v1 : v1, true);
            chunk->addVertex(reverse ? v0 : v2, true);
            ++count;
        }

    }
    return count;
}

QuadNodeHandler::QuadNodeHandler(Geometry * chunk, int * count){
    this->chunk = chunk;
    this->count = count;
}

QuadNodeTesselatorHandler::QuadNodeTesselatorHandler(Geometry * chunk, int * count) : QuadNodeHandler(chunk, count){

}

void QuadNodeTesselatorHandler::handle(OctreeNode* c0,OctreeNode* c1,OctreeNode* c2, bool sign) {
    if(c0 != NULL && c1 != NULL && c2!=NULL) {
	    *count += addTriangle(c0,c1,c2, chunk, sign);
    }
}

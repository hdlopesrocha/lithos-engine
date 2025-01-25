#include <bitset>
#include "math.hpp"
#include <bits/stdc++.h>

//      6-----7
//     /|    /|
//    4z+---5 |
//    | 2y--+-3
//    |/    |/
//    0-----1x

static std::vector<glm::ivec4> tessOrder;
static std::vector<glm::ivec2> tessEdge;
static std::vector<glm::vec2> tessTex;
static std::vector<glm::ivec4> texIndex;

static bool initialized = false;

Tesselator::Tesselator(Octree * tree) {
	this->tree = tree;

	if(!initialized) {
		tessOrder.push_back(glm::ivec4(0,1,3,2));tessEdge.push_back(glm::ivec2(0,4));texIndex.push_back(glm::ivec4(0,1,2,3));
		tessOrder.push_back(glm::ivec4(0,2,6,4));tessEdge.push_back(glm::ivec2(0,1));texIndex.push_back(glm::ivec4(0,1,2,3));
		tessOrder.push_back(glm::ivec4(0,4,5,1));tessEdge.push_back(glm::ivec2(0,2));texIndex.push_back(glm::ivec4(0,1,2,3));

		tessTex.push_back(glm::vec2(0,0));
		tessTex.push_back(glm::vec2(0,1));
		tessTex.push_back(glm::vec2(1,1));
		tessTex.push_back(glm::vec2(1,0));

		initialized = true;
	}
}

int addQuad(glm::ivec4 quad, std::vector<OctreeNode*> corners, Geometry * chunk, bool reverse) {
	OctreeNode* c0 = corners[quad[reverse ? 3:0]];
	OctreeNode* c1 = corners[quad[reverse ? 2:1]];
	OctreeNode* c2 = corners[quad[reverse ? 1:2]];
	OctreeNode* c3 = corners[quad[reverse ? 0:3]];


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

	if(c0!= c1 && c1 != c2){
		chunk->addVertex(v0, true);
		chunk->addVertex(v2, true);
		chunk->addVertex(v1, true);
		++count;
	}

	if(c0!= c3 && c3 != c2){
		chunk->addVertex(v0, true);
		chunk->addVertex(v3, true);
		chunk->addVertex(v2, true);
		++count;
	}
	return count;
}

OctreeNode * Tesselator::getChild(OctreeNode * node, int index){
	OctreeNode * child = node->children[index];
	return child != NULL ? child : NULL;
}

void * Tesselator::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
	if(tree->getHeight(cube)==tree->geometryLevel){
		Geometry * chunk = new Geometry();
		return chunk;
	} else if(tree->getHeight(cube)==0){
		Geometry * chunk = (Geometry*) context;
		std::vector<OctreeNode*> corners;
		// Get corners
		//corners.push_back(node);
		for(int i=0; i < 8; ++i) {
			glm::vec3 pos = cube.getCenter() - cube.getLength() * Octree::getShift(i);
			OctreeNode * n = tree->getNodeAt(pos,level);
			corners.push_back(n);
		}
	
		// Tesselate
		for(int k=0; k<tessOrder.size(); ++k){
			glm::ivec4 quad = tessOrder[k];
			glm::ivec3 texOrder = texIndex[k];
			glm::ivec2 edge = tessEdge[k];
		
			uint mask = node->mask;
			bool sign0 = (mask & (1 << edge[0])) != 0;
			bool sign1 = (mask & (1 << edge[1])) != 0;

			if(sign0 != sign1) {
				bool canDraw = true;
	
				for(int j=0; j<4 ; ++j) {
					OctreeNode * n = corners[quad[j]];
					if(n == NULL || n->mask == 0x0 || n->mask == 0xff) {
						canDraw = false;
						break;
					}
				}


				if(canDraw) {
					triangles += addQuad(quad, corners, chunk,  sign1);
				}
			}
		}
	}
	return context; 			 			
}

void Tesselator::after(int level, OctreeNode * node, BoundingCube cube, void * context) {
	if(tree->getHeight(cube)==tree->geometryLevel){
		Geometry * chunk = (Geometry*) context;
		node->info = chunk;
		node->infoType = 0;
	}		
	return;
}

bool Tesselator::test(int level, OctreeNode * node, BoundingCube cube, void * context) {			
	return node->mask!=0xff && tree->getHeight(cube) >= 0;
}


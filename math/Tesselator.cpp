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

static bool initialized = false;

Tesselator::Tesselator(Octree * tree, int * triangles, Geometry * chunk) {
	this->tree = tree;
	this->chunk = chunk;
	this->triangles = triangles;
	if(!initialized) {
		tessOrder.push_back(glm::ivec4(0,1,3,2));tessEdge.push_back(glm::ivec2(3,7));
		tessOrder.push_back(glm::ivec4(0,2,6,4));tessEdge.push_back(glm::ivec2(6,7));
		tessOrder.push_back(glm::ivec4(0,4,5,1));tessEdge.push_back(glm::ivec2(5,7));

		initialized = true;
	}
}

int addQuad(std::vector<OctreeNode*> quad, Geometry * chunk, bool reverse) {
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


	if(c0!= c1 && c1 != c2 && c0!=c2){
		chunk->addVertex(v0, true);
		chunk->addVertex(v2, true);
		chunk->addVertex(v1, true);
		++count;
	}

	if(c0!= c3 && c3 != c2 && c0!=c2){
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

void * Tesselator::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
	if(tree->getHeight(cube)==0){
		std::vector<OctreeNode*> corners = tree->getNodeCorners(cube, level, true, 1);
		// Tesselate
		for(int k=0; k<tessOrder.size(); ++k){
			glm::ivec4 quad = tessOrder[k];
			glm::ivec2 edge = tessEdge[k];
		
			uint mask = node->mask;
			bool sign0 = (mask & (1 << edge[0])) != 0;
			bool sign1 = (mask & (1 << edge[1])) != 0;

			if(sign0 != sign1) {
				std::vector<OctreeNode*> quadNodes = tree->getQuadNodes(corners, quad);	
				if(quadNodes.size() == 4) {
					*triangles += addQuad(quadNodes, chunk,  sign1);
				}
			}
		}
	}
	return context;
}

void Tesselator::after(int level, OctreeNode * node, BoundingCube cube, void * context) {
	return;
}

bool Tesselator::test(int level, OctreeNode * node, BoundingCube cube, void * context) {			
	return node->solid != ContainmentType::Contains && tree->getHeight(cube) >= 0;
}

std::vector<int> Tesselator::getOrder(OctreeNode * node) {
	std::vector<int> nodes;
	for(int i = 0 ; i < 8 ; ++i) {
		nodes.push_back(i);
	}
	return nodes;
}
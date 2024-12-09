#include <bitset>
#include "math.hpp"

//      6-----7
//     /|    /|
//    4-+---5 |
//    | 2---+-3
//    |/    |/
//    0-----1

static std::vector<glm::ivec3> tessOrder;
static std::vector<glm::ivec2> tessEdge;
static std::vector<glm::vec2> tessTex;
static bool initialized = false;

Tesselator::Tesselator(Octree * tree) {
	this->tree = tree;

	if(!initialized) {
		tessOrder.push_back(glm::ivec3(0,3,1));tessEdge.push_back(glm::ivec2(0b00001000,0b10000000));
		tessOrder.push_back(glm::ivec3(0,2,3));tessEdge.push_back(glm::ivec2(0b00001000,0b10000000));
		tessOrder.push_back(glm::ivec3(0,6,2));tessEdge.push_back(glm::ivec2(0b01000000,0b10000000));
		tessOrder.push_back(glm::ivec3(0,4,6));tessEdge.push_back(glm::ivec2(0b01000000,0b10000000));
		tessOrder.push_back(glm::ivec3(0,5,4));tessEdge.push_back(glm::ivec2(0b00100000,0b10000000));
		tessOrder.push_back(glm::ivec3(0,1,5));tessEdge.push_back(glm::ivec2(0b00100000,0b10000000));

		tessOrder.push_back(glm::ivec3(0,1,3));tessEdge.push_back(glm::ivec2(0b10000000,0b00001000));
		tessOrder.push_back(glm::ivec3(0,3,2));tessEdge.push_back(glm::ivec2(0b10000000,0b00001000));
		tessOrder.push_back(glm::ivec3(0,2,6));tessEdge.push_back(glm::ivec2(0b10000000,0b01000000));
		tessOrder.push_back(glm::ivec3(0,6,4));tessEdge.push_back(glm::ivec2(0b10000000,0b01000000));
		tessOrder.push_back(glm::ivec3(0,4,5));tessEdge.push_back(glm::ivec2(0b10000000,0b00100000));
		tessOrder.push_back(glm::ivec3(0,5,1));tessEdge.push_back(glm::ivec2(0b10000000,0b00100000)); 

		tessTex.push_back(glm::vec2(0,0));
		tessTex.push_back(glm::vec2(0,1));
		tessTex.push_back(glm::vec2(1,0));

		initialized = true;
	}

}

void Tesselator::addVertex(Vertex vertex){
	if(!compactMap.count(vertex.toString())) {
		compactMap[vertex.toString()] = compactMap.size();
		vertices.push_back(vertex); 
	}
	int idx = compactMap[vertex.toString()];
	indices.push_back(idx);
//	std::cout << "i=" << idx << std::endl;
}

void Tesselator::iterate(int level, OctreeNode * node, BoundingCube cube) {			
	if(node->leaf){
		std::vector<OctreeNode*> corners;
		// Get corners
		corners.push_back(node);
		for(int i=1; i < 8; ++i) {
			glm::vec3 pos = cube.getCenter() + Octree::getShift(i)*cube.getLength();
			OctreeNode * n = tree->getNodeAt(pos,level);
			corners.push_back(n);
		}
		uint nodeMask = 0x0;

		for(int i = 0 ; i < 8 ; ++i) {
			OctreeNode * c = corners[i];
			nodeMask |= c!=NULL && (c->solid == ContainmentType::Intersects || c->solid == ContainmentType::Contains) ? (1 << i) : 0;
		}

		// Tesselate
		for(int k=0; k<tessOrder.size(); ++k){
			glm::ivec3 triOrder = tessOrder[k];
			glm::ivec2 tess = tessEdge[k];
			//if((nodeMask & tess[0]) && !(nodeMask & tess[1])){

				bool accepted = true;
				for(int j=0; j < 3; ++j){
					OctreeNode * n = corners[triOrder[j]];

					if(n == NULL || n->solid != ContainmentType::Intersects){
						accepted = false;
						break;
					}
				}		
				if(accepted) {
					for(int j=0; j < 3; ++j){
						OctreeNode * n = corners[triOrder[j]];
						Vertex vtx = n->vertex;
						vtx.texCoord = tessTex[j];
						addVertex(vtx);
					}
				}
			//}
		}
	} 			 			
}



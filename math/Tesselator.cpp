#include <bitset>
#include "math.hpp"

//      6-----7
//     /|    /|
//    4z+---5 |
//    | 2y--+-3
//    |/    |/
//    0-----1x

static std::vector<glm::ivec4> tessOrder;
static std::vector<uint> tessEdge;
static std::vector<glm::vec2> tessTex;
static bool initialized = false;

Tesselator::Tesselator(Octree * tree) {
	this->tree = tree;

	if(!initialized) {
		tessOrder.push_back(glm::ivec4(0,2,1,3));tessEdge.push_back(0b11110000);
		tessOrder.push_back(glm::ivec4(1,2,3,0));tessEdge.push_back(0b11110000);
		tessOrder.push_back(glm::ivec4(0,4,2,6));tessEdge.push_back(0b10101010);
		tessOrder.push_back(glm::ivec4(2,4,6,0));tessEdge.push_back(0b10101010);
		tessOrder.push_back(glm::ivec4(0,1,4,5));tessEdge.push_back(0b11001100);
		tessOrder.push_back(glm::ivec4(4,1,5,0));tessEdge.push_back(0b11001100);

		tessTex.push_back(glm::vec2(0,0));
		tessTex.push_back(glm::vec2(0,1));
		tessTex.push_back(glm::vec2(1,0));


		for(int i=0 ; i < 8; ++i) {
			glm::vec3 v = Octree::getShift(i);
			std::cout << "" << i << ": " << v.x << " "<< v.y << " "<< v.z << std::endl; 
		}
		initialized = true;
	}

}

void Tesselator::addVertex(Vertex vertex){
	std::string key = vertex.toKey();
	if(!compactMap.count(key)) {
		compactMap[key] = compactMap.size();
		vertices.push_back(vertex); 
	}
	int idx = compactMap[key];
	indices.push_back(idx);
//	std::cout << "i=" << idx << std::endl;
}

void Tesselator::iterate(int level, OctreeNode * node, BoundingCube cube) {			
	if(node->leaf && node->solid == ContainmentType::Intersects){
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
			nodeMask |= c!=NULL && (c->solid == ContainmentType::Contains) ? (1 << i) : 0;
		}

		// Tesselate
		for(int k=0; k<tessOrder.size(); ++k){
			glm::ivec4 triOrder = tessOrder[k];
//			if((nodeMask & tess[0]) && !(nodeMask & tess[1])){
				bool accepted = true;
				for(int j=0; j < 4; ++j){
					OctreeNode * n = corners[triOrder[j]];
					if(n == NULL || n->solid != ContainmentType::Intersects){
						accepted = false;
						break;
					}
				}		
				if(accepted) {
					uint m = tessEdge[k];
					bool empty = m & nodeMask;
					
					for(int j=0; j < 3; ++j){
						int l = empty ? 2-j : j;
						OctreeNode * n = corners[triOrder[l]];
						Vertex vtx = n->vertex;
						vtx.texCoord = tessTex[l];
						addVertex(vtx);
					}
				}
//			}
		}
	} 			 			
}



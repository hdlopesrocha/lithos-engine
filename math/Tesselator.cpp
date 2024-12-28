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
static std::vector<glm::ivec4> texIndex;

static bool initialized = false;

Tesselator::Tesselator(Octree * tree) {
	this->tree = tree;

	if(!initialized) {
		tessOrder.push_back(glm::ivec4(0,2,3,1));tessEdge.push_back(0b11110000);texIndex.push_back(glm::ivec4(0,1,2,3));
		tessOrder.push_back(glm::ivec4(0,4,6,2));tessEdge.push_back(0b10101010);texIndex.push_back(glm::ivec4(0,1,2,3));
		tessOrder.push_back(glm::ivec4(0,1,5,4));tessEdge.push_back(0b11001100);texIndex.push_back(glm::ivec4(0,1,2,3));

		tessTex.push_back(glm::vec2(0,0));
		tessTex.push_back(glm::vec2(0,1));
		tessTex.push_back(glm::vec2(1,1));
		tessTex.push_back(glm::vec2(1,0));


		for(int i=0 ; i < 8; ++i) {
			glm::vec3 v = Octree::getShift(i);
			std::cout << "" << i << ": " << v.x << " "<< v.y << " "<< v.z << std::endl; 
		}
		initialized = true;
	}

}

Vertex * Tesselator::addVertex(Vertex vertex){
	std::string key = vertex.toKey();
	if(!compactMap.count(key)) {
		compactMap[key] = compactMap.size();
		vertices.push_back(vertex); 
	}
	int idx = compactMap[key];
	indices.push_back(idx);
	return &(vertices[idx]);
}


int triplanarPlane(glm::vec3 position, glm::vec3 normal) {
    glm::vec3 absNormal = glm::abs(normal);
    if (absNormal.x > absNormal.y && absNormal.x > absNormal.z) {
        return 0;
    } else if (absNormal.y > absNormal.x && absNormal.y > absNormal.z) {
        return 1;
    } else {
        return 2;
    }
}

glm::vec2 triplanarMapping(glm::vec3 position, int plane) {
    if (plane == 0) {
        return glm::vec2(position.y,position.z);
    } else if (plane ==1) {
        return glm::vec2(position.z, position.x);
    } else {
        return glm::vec2(position.x, position.y);
    }
}


int Tesselator::iterate(int level, OctreeNode * node, BoundingCube cube) {			
	if(tree->getHeight(cube)==0 && node->solid == ContainmentType::Intersects){
		std::vector<OctreeNode*> corners;
		// Get corners
		corners.push_back(node);
		for(int i=1; i < 8; ++i) {
			glm::vec3 pos = cube.getCenter() + Octree::getShift(i)*cube.getLength();
			OctreeNode * n = tree->getNodeAt(pos,level);
			corners.push_back(n);
		}
	
		uint nodeMask = 0x0;
		int plane = triplanarPlane(node->vertex.position, node->vertex.normal);

		for(int i = 0 ; i < 8 ; ++i) {
			OctreeNode * c = corners[i];
			nodeMask |= c!=NULL && (c->solid == ContainmentType::Contains) ? (1 << i) : 0;
			if(c!=NULL) {
				Vertex * vtx = &(c->vertex);
				vtx->texCoord = triplanarMapping(vtx->position, plane)*0.1f;
			}
		}

		// Tesselate
		for(int k=0; k<tessOrder.size(); ++k){
			glm::ivec4 triangle = tessOrder[k];
			glm::ivec3 texOrder = texIndex[k];
			uint m = tessEdge[k];
			
//			if((nodeMask & tess[0]) && !(nodeMask & tess[1])){
				bool isSurface = true;
				bool empty = m & nodeMask;		            

				for(int j=0; j < 4; ++j) {
					OctreeNode * n = corners[triangle[j]];
					if(n == NULL || n->solid != ContainmentType::Intersects){
						isSurface = false;
						break;
					}
				}	
				if(isSurface) {
	
					glm::ivec4 order = glm::ivec4();
					for(int j=0; j < 4; ++j){
						int l = empty ? 3-j : j;
						order[j]=l;
					}

						//glm::ivec2 t = tessTex[texOrder[order[j]]]; 

					Vertex v0 = corners[triangle[order[0]]]->vertex;
					Vertex v1 = corners[triangle[order[1]]]->vertex;
					Vertex v2 = corners[triangle[order[2]]]->vertex;
					Vertex v3 = corners[triangle[order[3]]]->vertex;

					addVertex(v0);
					addVertex(v1);
					addVertex(v2);

					addVertex(v0);
					addVertex(v2);
					addVertex(v3);

					glm::vec3 edge1 = v1.position - v0.position;
					glm::vec3 edge2 = v3.position - v0.position;

 					glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
  				    glm::vec2 deltaUV2 = v3.texCoord - v0.texCoord;

				}
//			}
		}
	}
	return 1; 			 			
}



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
		tessOrder.push_back(glm::ivec4(0,1,3,2));tessEdge.push_back(0b11110000);texIndex.push_back(glm::ivec4(0,1,2,3));
		tessOrder.push_back(glm::ivec4(0,2,6,4));tessEdge.push_back(0b10101010);texIndex.push_back(glm::ivec4(0,1,2,3));
		tessOrder.push_back(glm::ivec4(0,4,5,1));tessEdge.push_back(0b11001100);texIndex.push_back(glm::ivec4(0,1,2,3));

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
        return normal.x > 0 ? 0 : 1;
    } else if (absNormal.y > absNormal.x && absNormal.y > absNormal.z) {
        return normal.y > 0 ? 2 : 3;
    } else {
        return normal.z > 0 ? 4 : 5;
    }
}

glm::vec2 triplanarMapping(glm::vec3 position, int plane) {
    switch (plane) {
        case 0: return glm::vec2(-position.z, -position.y);
        case 1: return glm::vec2(position.z, -position.y);
        case 2: return glm::vec2(position.x, position.z);
        case 3: return glm::vec2(position.x, -position.z);
        case 4: return glm::vec2(position.x, -position.y);
        case 5: return glm::vec2(-position.x, -position.y);
        default: return glm::vec2(0.0,0.0);
    }
}

void Tesselator::normalize() {
	for(int i=0; i < vertices.size() ; ++i) {
		Vertex v = vertices[i];
		v.normal = glm::normalize(v.normal);
		vertices[i] = v;
	}
}

void smooth(Vertex * v, glm::vec3 n, glm::vec3 t, glm::vec3 b) {
	v->normal += n;
	v->tangent += t;
	v->bitangent += b;
}

int Tesselator::iterate(int level, OctreeNode * node, BoundingCube cube) {		
	if(tree->getHeight(cube)==0 && node->solid == ContainmentType::Intersects){
		std::vector<OctreeNode*> corners;
		// Get corners
		corners.push_back(node);
		for(int i=1; i < 8; ++i) {
			glm::vec3 pos = cube.getCenter() + Octree::getShift(i)*(cube.getLength());
			OctreeNode * n = tree->getNodeAt(pos,level);
			corners.push_back(n);
		}
	
		uint emptyMask = 0x0;
		uint containmentMask = 0x0;
		uint surfaceMask = 0x0;
		int plane = triplanarPlane(node->vertex.position, node->vertex.normal);

		for(int i = 0 ; i < 8 ; ++i) {
			OctreeNode * c = corners[i];
			emptyMask |= c==NULL || (c->solid == ContainmentType::Disjoint) ? (1 << i) : 0;
			surfaceMask |= c!=NULL && (c->solid == ContainmentType::Intersects) ? (1 << i) : 0;
			containmentMask |= c!=NULL && (c->solid == ContainmentType::Contains) ? (1 << i) : 0;
		}

	

		// Tesselate
		for(int k=0; k<tessOrder.size(); ++k){
			glm::ivec4 triangle = tessOrder[k];
			glm::ivec3 texOrder = texIndex[k];
			uint expectedMask = tessEdge[k];
			
			uint expectedSurfaceMask = 0x0;
			for(int l=0; l<4 ; ++l) {
				int m = triangle[l];
				expectedSurfaceMask |= (1 << m);
			}

			if((surfaceMask & expectedSurfaceMask) == expectedSurfaceMask && (k < 3 ? (emptyMask & expectedMask) : (containmentMask & expectedMask))) {
				Vertex v0 = corners[triangle[0]]->vertex;
				Vertex v1 = corners[triangle[1]]->vertex;
				Vertex v2 = corners[triangle[2]]->vertex;
				Vertex v3 = corners[triangle[3]]->vertex;


				glm::vec3 edge1 = v1.position - v0.position;
				glm::vec3 edge2 = v3.position - v0.position;

				glm::vec3 normal = glm::cross(edge2,edge1);

				float scale = 0.1;
				int plane = triplanarPlane(v0.position, normal);
				v0.texCoord = triplanarMapping(v0.position, plane)*scale;
				v1.texCoord = triplanarMapping(v1.position, plane)*scale;
				v2.texCoord = triplanarMapping(v2.position, plane)*scale;
				v3.texCoord = triplanarMapping(v3.position, plane)*scale;	

				glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
				glm::vec2 deltaUV2 = v3.texCoord - v0.texCoord;

			    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);


				glm::vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
				tangent = glm::normalize(tangent);
    			tangent = glm::normalize(tangent - glm::dot(tangent, normal) * normal);

				glm::vec3 bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);
				bitangent = glm::normalize(bitangent);
    			bitangent = glm::normalize(bitangent - glm::dot(bitangent, normal) * normal);


				smooth(addVertex(v0), normal, tangent, bitangent);
				smooth(addVertex(v2), normal, tangent, bitangent);
				smooth(addVertex(v1), normal, tangent, bitangent);

				smooth(addVertex(v0), normal, tangent, bitangent);
				smooth(addVertex(v3), normal, tangent, bitangent);
				smooth(addVertex(v2), normal, tangent, bitangent);


			}
		}
	}
	return 1; 			 			
}



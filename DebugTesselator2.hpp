#include <bitset>
#include "math/math.hpp"

//      6-----7
//     /|    /|
//    4z+---5 |
//    | 2y--+-3
//    |/    |/
//    0-----1x
static bool initializedDebug2 = false;


static std::vector<glm::ivec4> tessOrder2;
static std::vector<uint> tessEdge2;
static std::vector<glm::vec2> tessTex2;
static std::vector<glm::ivec4> texIndex2;


class DebugTesselator2 : public IteratorHandler{
	Octree * tree;
	Geometry chunk;



	public: 

	DebugTesselator2(Octree * tree) {
		this->tree = tree;

		if(!initializedDebug2) {
			tessOrder2.push_back(glm::ivec4(0,1,3,2));tessEdge2.push_back(0b11110000);texIndex2.push_back(glm::ivec4(0,1,2,3));
			tessOrder2.push_back(glm::ivec4(0,2,6,4));tessEdge2.push_back(0b10101010);texIndex2.push_back(glm::ivec4(0,1,2,3));
			tessOrder2.push_back(glm::ivec4(0,4,5,1));tessEdge2.push_back(0b11001100);texIndex2.push_back(glm::ivec4(0,1,2,3));

			tessOrder2.push_back(glm::ivec4(0,2,3,1));tessEdge2.push_back(0b11110000);texIndex2.push_back(glm::ivec4(0,1,2,3));
			tessOrder2.push_back(glm::ivec4(0,4,6,2));tessEdge2.push_back(0b10101010);texIndex2.push_back(glm::ivec4(0,1,2,3));
			tessOrder2.push_back(glm::ivec4(0,1,5,4));tessEdge2.push_back(0b11001100);texIndex2.push_back(glm::ivec4(0,1,2,3));

			tessTex2.push_back(glm::vec2(0,0));
			tessTex2.push_back(glm::vec2(0,1));
			tessTex2.push_back(glm::vec2(1,1));
			tessTex2.push_back(glm::vec2(1,0));

			for(int i=0 ; i < 8; ++i) {
				glm::vec3 v = Octree::getShift(i);
				std::cout << "" << i << ": " << v.x << " "<< v.y << " "<< v.z << std::endl; 
			}
			initializedDebug2 = true;
		}
	}



	void * before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
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
	
			for(int i = 0 ; i < 8 ; ++i) {
				OctreeNode * c = corners[i];
				emptyMask |= c==NULL || (c->solid == ContainmentType::Disjoint) ? (1 << i) : 0;
				surfaceMask |= c!=NULL && (c->solid == ContainmentType::Intersects) ? (1 << i) : 0;
				containmentMask |= c!=NULL && (c->solid == ContainmentType::Contains) ? (1 << i) : 0;
			}

		

			// Tesselate
			for(int k=0; k<tessOrder2.size(); ++k){
				glm::ivec4 triangle = tessOrder2[k];
				glm::ivec3 texOrder = texIndex2[k];
				uint expectedMask = tessEdge2[k];
				
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

				glm::vec3 normal = glm::normalize( glm::cross(edge2,edge1))*0.2f;
				v0.position += normal;
				v1.position += normal;
				v2.position += normal;
				v3.position += normal;


					v0.texCoord = tessTex2[0];
					v1.texCoord = tessTex2[1];
					v2.texCoord = tessTex2[2];
					v3.texCoord = tessTex2[3];

					v0.texIndex = 1;
					v1.texIndex = 1;
					v2.texIndex = 1;
					v3.texIndex = 1;


					chunk.addVertex(v0);
					chunk.addVertex(v2);
					chunk.addVertex(v1);

					chunk.addVertex(v0);
					chunk.addVertex(v3);
					chunk.addVertex(v2);


				}
			}
		}
		return NULL; 			 			
	}

		int after(int level, OctreeNode * node, BoundingCube cube, void * context) {			
			return 1;
		}
};
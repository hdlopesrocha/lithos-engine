#include <bitset>
#include "math/math.hpp"

//      6-----7
//     /|    /|
//    4z+---5 |
//    | 2y--+-3
//    |/    |/
//    0-----1x

static std::vector<glm::ivec3> tessOrderDebug;
static std::vector<glm::vec2> tessTexDebug;
static bool initializedDebug = false;


class DebugTesselator : public TesselatorHandler{
	Octree * tree;
	std::map <std::string, int> compactMap;

	public: 

		DebugTesselator(Octree * tree) {
			this->tree = tree;

			if(!initializedDebug) {
				tessOrderDebug.push_back(glm::ivec3(0,2,3));
				tessOrderDebug.push_back(glm::ivec3(3,1,0));
				tessOrderDebug.push_back(glm::ivec3(0,4,6));
				tessOrderDebug.push_back(glm::ivec3(6,2,0));
				tessOrderDebug.push_back(glm::ivec3(0,1,5));
				tessOrderDebug.push_back(glm::ivec3(5,4,0));

				tessOrderDebug.push_back(glm::ivec3(4,5,7));
				tessOrderDebug.push_back(glm::ivec3(7,6,4));
				tessOrderDebug.push_back(glm::ivec3(1,3,7));
				tessOrderDebug.push_back(glm::ivec3(7,5,1));
				tessOrderDebug.push_back(glm::ivec3(2,6,7));
				tessOrderDebug.push_back(glm::ivec3(7,3,2));

				tessTexDebug.push_back(glm::vec2(0,0));
				tessTexDebug.push_back(glm::vec2(0,1));
				tessTexDebug.push_back(glm::vec2(1,1));


				for(int i=0 ; i < 8; ++i) {
					glm::vec3 v = Octree::getShift(i);
					std::cout << "" << i << ": " << v.x << " "<< v.y << " "<< v.z << std::endl; 
				}
				initializedDebug = true;
			}

		}

		void addVertex(Vertex vertex){
			std::string key = vertex.toString();
			if(!compactMap.count(key)) {
				compactMap[key] = compactMap.size();
				vertices.push_back(vertex); 
			}
			int idx = compactMap[key];
			indices.push_back(idx);
		//	std::cout << "i=" << idx << std::endl;
		}

		void iterate(int level, OctreeNode * node, BoundingCube cube) {			
			if(node->leaf && node->solid == ContainmentType::Intersects || node->solid == ContainmentType::Contains){
				std::vector<glm::vec3> corners;
				// Get corners
				for(int i=0; i < 8; ++i) {
					glm::vec3 pos = cube.getMin() + Octree::getShift(i)*cube.getLength();
					corners.push_back(pos);
				}
			
				// Tesselate
				for(int k=0; k<tessOrderDebug.size(); ++k){
					glm::ivec3 triOrder = tessOrderDebug[k];
			
					for(int j=0; j < 3; ++j){
						Vertex vtx;
						vtx.pos = corners[triOrder[j]];
						vtx.texCoord = tessTexDebug[j];
						vtx.texIndex = node->solid == ContainmentType::Contains ? 9 : 1;
						vtx.normal = glm::vec3(0.0);
						addVertex(vtx);
					}
				}
			} 			 			
		}



};


#include <bitset>
#include "math.hpp"

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
		tessOrder.push_back(glm::ivec4(0,1,3,2));tessEdge.push_back(glm::ivec2(3,7));texIndex.push_back(glm::ivec4(0,1,2,3));
		tessOrder.push_back(glm::ivec4(0,2,6,4));tessEdge.push_back(glm::ivec2(6,7));texIndex.push_back(glm::ivec4(0,1,2,3));
		tessOrder.push_back(glm::ivec4(0,4,5,1));tessEdge.push_back(glm::ivec2(5,7));texIndex.push_back(glm::ivec4(0,1,2,3));

		tessOrder.push_back(glm::ivec4(0,2,3,1));tessEdge.push_back(glm::ivec2(7,3));texIndex.push_back(glm::ivec4(0,1,2,3));
		tessOrder.push_back(glm::ivec4(0,4,6,2));tessEdge.push_back(glm::ivec2(7,6));texIndex.push_back(glm::ivec4(0,1,2,3));
		tessOrder.push_back(glm::ivec4(0,1,5,4));tessEdge.push_back(glm::ivec2(7,5));texIndex.push_back(glm::ivec4(0,1,2,3));

		tessTex.push_back(glm::vec2(0,0));
		tessTex.push_back(glm::vec2(0,1));
		tessTex.push_back(glm::vec2(1,1));
		tessTex.push_back(glm::vec2(1,0));

		initialized = true;
	}
}

void * Tesselator::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
	if(tree->getHeight(cube)==tree->geometryLevel){
		Geometry * chunk = new Geometry();
		return chunk;
	} else if(tree->getHeight(cube)==0){
		Geometry * chunk = (Geometry*) context;
		std::vector<OctreeNode*> corners;
		// Get corners
		corners.push_back(node);
		for(int i=1; i < 8; ++i) {
			glm::vec3 pos = cube.getCenter() + cube.getLength() * Octree::getShift(i);
			OctreeNode * n = tree->getNodeAt(pos,level);
			corners.push_back(n);
		}
	
		// Tesselate
		for(int k=0; k<tessOrder.size(); ++k){
			glm::ivec4 triangle = tessOrder[k];
			glm::ivec3 texOrder = texIndex[k];
			glm::ivec2 edge = tessEdge[k];
		
			uint mask = node->mask;
			if((mask & (1 << edge[0])) && !(mask & (1 << edge[1]))) {
				bool canDraw = true;
				for(int j=0; j<4 ; ++j) {
					OctreeNode * n = corners[triangle[j]];
					if(n == NULL || n->solid != ContainmentType::Intersects) {
						canDraw = false;
					}
				}
				if(canDraw) {
					Vertex v0 = corners[triangle[0]]->vertex;
					Vertex v1 = corners[triangle[1]]->vertex;
					Vertex v2 = corners[triangle[2]]->vertex;
					Vertex v3 = corners[triangle[3]]->vertex;

					float scale = 0.1;
					int plane = Math::triplanarPlane(v0.position, v0.normal);
					v0.texCoord = Math::triplanarMapping(v0.position, plane)*scale;
					v1.texCoord = Math::triplanarMapping(v1.position, plane)*scale;
					v2.texCoord = Math::triplanarMapping(v2.position, plane)*scale;
					v3.texCoord = Math::triplanarMapping(v3.position, plane)*scale;	

					chunk->addVertex(v0, true);
					chunk->addVertex(v2, true);
					chunk->addVertex(v1, true);

					chunk->addVertex(v0, true);
					chunk->addVertex(v3, true);
					chunk->addVertex(v2, true);
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
	return true;
}


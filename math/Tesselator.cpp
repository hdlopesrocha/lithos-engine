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

		initialized = true;
	}
}

void smooth(Vertex * v, glm::vec3 n) {
	v->normal += n;
}

void * Tesselator::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
	if(tree->getHeight(cube)==tree->geometryLevel){
		Geometry * chunk = new Geometry();
		return chunk;
	} else if(tree->getHeight(cube)==0 && node->solid == ContainmentType::Intersects){
		Geometry * chunk = (Geometry*) context;
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
		int plane = Math::triplanarPlane(node->vertex.position, node->vertex.normal);

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
				int plane = Math::triplanarPlane(v0.position, normal);
				v0.texCoord = Math::triplanarMapping(v0.position, plane)*scale;
				v1.texCoord = Math::triplanarMapping(v1.position, plane)*scale;
				v2.texCoord = Math::triplanarMapping(v2.position, plane)*scale;
				v3.texCoord = Math::triplanarMapping(v3.position, plane)*scale;	


				smooth(chunk->addVertex(v0), normal);
				smooth(chunk->addVertex(v2), normal);
				smooth(chunk->addVertex(v1), normal);

				smooth(chunk->addVertex(v0), normal);
				smooth(chunk->addVertex(v3), normal);
				smooth(chunk->addVertex(v2), normal);


			}
		}
	}
	return context; 			 			
}

void Tesselator::after(int level, OctreeNode * node, BoundingCube cube, void * context) {
	if(tree->getHeight(cube)==tree->geometryLevel){
		Geometry * chunk = (Geometry*) context;
		chunk->normalize();
		node->info = chunk;
		node->infoType = 0;

		//delete chunk;	
	}		
	return;
}


bool Tesselator::test(int level, OctreeNode * node, BoundingCube cube, void * context) {			
	return 1;
}


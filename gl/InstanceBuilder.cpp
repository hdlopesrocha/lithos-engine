#include <bitset>
#include "../math/math.hpp"
#include "gl.hpp"
#include <glm/gtx/norm.hpp> 

InstanceBuilder::InstanceBuilder(Octree * tree,  int drawableType, int geometryLevel, int * triangles) {
	this->tree = tree;
	this->drawableType = drawableType;
	this->geometryLevel = geometryLevel;
	this->triangles = triangles;
}


OctreeNode * InstanceBuilder::getChild(OctreeNode * node, int index){
	return node->children[index];
}

void * InstanceBuilder::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
	if(tree->getHeight(cube)==0){
		
		Vertex * v = &node->vertex;
		if(v->brushIndex == 2) {
			glm::mat4 model(1.0);

			OctreeNode * corners[8];
			tree->getNodeCorners(cube, level, 0, 1, corners);

			QuadNodeHandler handler(&chunk, triangles);
			//std::cout << std::to_string(v->position.x) << "_"<< std::to_string(v->position.y) << "_"<< std::to_string(v->position.z) << std::endl;
			tree->getQuadNodes(corners, &handler, triangles) ;
			
			int radius = 4;
			// TODO: interpolate using triangles
			for(int x=0 ; x < radius; ++x) {
				for(int z=0 ; z < radius; ++z) {
					model = glm::translate(glm::mat4(1.0), v->position+glm::vec3(x,0,z));
					matrices.push_back(model);
				}
			}
		}
		
		return node;
	}
	return NULL; 			 			
}

void InstanceBuilder::after(int level, OctreeNode * node, BoundingCube cube, void * context) {			
	return;
}

bool InstanceBuilder::test(int level, OctreeNode * node, BoundingCube cube, void * context) {	
	return context == NULL;
}


void InstanceBuilder::getOrder(OctreeNode * node, BoundingCube cube, int * order){
	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}


/*
void OctreeRenderer::getOrder(OctreeNode * node, BoundingCube cube, int * order){
	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}
*/
#include <bitset>
#include "../math/math.hpp"
#include "gl.hpp"
#include <glm/gtx/norm.hpp> 

InstanceBuilder::InstanceBuilder(Octree * tree,  int drawableType, int geometryLevel) {
	this->tree = tree;
	this->drawableType = drawableType;
	this->geometryLevel = geometryLevel;
}


OctreeNode * InstanceBuilder::getChild(OctreeNode * node, int index){
	return node->children[index];
}

void * InstanceBuilder::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
	if(tree->getHeight(cube)==0){
		
		Vertex * v = &node->vertex;
		if(v->brushIndex == 2) {
			glm::mat4 model(1.0);
			//std::cout << std::to_string(v->position.x) << "_"<< std::to_string(v->position.y) << "_"<< std::to_string(v->position.z) << std::endl;
			int radius = 4;
			
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
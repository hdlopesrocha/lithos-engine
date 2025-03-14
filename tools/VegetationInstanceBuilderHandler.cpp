#include "tools.hpp"


VegetationInstanceBuilderHandler::VegetationInstanceBuilderHandler(Octree * tree, long * count) : InstanceBuilderHandler(tree, count){

}

void VegetationInstanceBuilderHandler::handle(OctreeNode *node, const BoundingCube &cube, int level, InstanceGeometry * pre){
	Vertex * vertex = &node->vertex;
	if(vertex->brushIndex == 2) { 
		OctreeNodeTriangleInstanceBuilder handler(pre->geometry, count , &pre->instances, 3);
		tree->handleQuadNodes(cube, level, *node, &handler);
	}
}
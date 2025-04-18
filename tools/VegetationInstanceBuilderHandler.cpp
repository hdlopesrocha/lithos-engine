#include "tools.hpp"


VegetationInstanceBuilderHandler::VegetationInstanceBuilderHandler(Octree * tree, long * count, int pointsPerTriangle, float scale) : InstanceBuilderHandler(tree, count){
	this->pointsPerTriangle = pointsPerTriangle;
	this->scale = scale;
}

void VegetationInstanceBuilderHandler::handle(OctreeNodeData &data, InstanceGeometry * pre){
	Vertex * vertex = &data.node->vertex;
	if(vertex->brushIndex == 2) { 
		VegetationInstanceBuilder handler(pre->geometry, count , &pre->instances, pointsPerTriangle, scale);
		tree->handleQuadNodes(data, &handler, false);
	}
}
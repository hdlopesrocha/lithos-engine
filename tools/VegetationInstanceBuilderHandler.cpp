#include "tools.hpp"


VegetationInstanceBuilderHandler::VegetationInstanceBuilderHandler(Octree * tree, int pointsPerTriangle, float scale) : InstanceBuilderHandler(){
	this->pointsPerTriangle = pointsPerTriangle;
	this->scale = scale;
	this->tree = tree;
}

void VegetationInstanceBuilderHandler::handle(OctreeNodeData &data, InstanceGeometry * pre){
	Vertex * vertex = &data.node->vertex;
	if(vertex->brushIndex == 2) { 
		long count = 0;
		VegetationInstanceBuilder handler(pre->geometry, &count , &pre->instances, pointsPerTriangle, scale);
		tree->handleQuadNodes(data, &handler, false);
	}
}
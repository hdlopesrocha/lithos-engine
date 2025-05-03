#include "tools.hpp"


VegetationInstanceBuilderHandler::VegetationInstanceBuilderHandler(Octree * tree, float pointsPerArea, float scale) : InstanceBuilderHandler(){
	this->pointsPerArea = pointsPerArea;
	this->scale = scale;
	this->tree = tree;
}

void VegetationInstanceBuilderHandler::handle(OctreeNodeData &data, std::vector<InstanceData> * instances){
	Vertex * vertex = &data.node->vertex;
	if(data.node->isLeaf() && vertex->brushIndex == 2) { 
		long count = 0;
		VegetationInstanceBuilder handler(&count , instances, pointsPerArea, scale);
		tree->handleQuadNodes(data, &handler, false);
	}
}
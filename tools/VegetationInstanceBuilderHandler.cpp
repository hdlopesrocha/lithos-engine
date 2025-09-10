#include "tools.hpp"


VegetationInstanceBuilderHandler::VegetationInstanceBuilderHandler(float pointsPerArea, float scale) {
	this->pointsPerArea = pointsPerArea;
	this->scale = scale;
}

void VegetationInstanceBuilderHandler::handle(Octree * tree, OctreeNodeData &data, std::vector<InstanceData> * instances, ChunkContext * context){
	Vertex * vertex = &data.node->vertex;
	if(data.node->isLeaf() && vertex->brushIndex == 2) { 
		long count = 0;
		VegetationInstanceBuilder handler(&count , instances, pointsPerArea, scale);
		tree->handleQuadNodes(data, data.node->sdf, &handler, false, context);
	}
}
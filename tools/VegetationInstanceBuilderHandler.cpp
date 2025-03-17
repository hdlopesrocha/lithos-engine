#include "tools.hpp"


VegetationInstanceBuilderHandler::VegetationInstanceBuilderHandler(Octree * tree, long * count) : InstanceBuilderHandler(tree, count){

}

void VegetationInstanceBuilderHandler::handle(OctreeNodeData &data, InstanceGeometry * pre){
	Vertex * vertex = &data.node->vertex;
	if(vertex->brushIndex == 2) { 
		VegetationInstanceBuilder handler(pre->geometry, count , &pre->instances, 3);
		tree->handleQuadNodes(data, &handler);
	}
}
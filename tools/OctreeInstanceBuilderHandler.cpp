#include "tools.hpp"


OctreeInstanceBuilderHandler::OctreeInstanceBuilderHandler() {

}

void OctreeInstanceBuilderHandler::handle(Octree * tree, OctreeNodeData &data, std::vector<DebugInstanceData> * instances){
	if(data.node->isSimplified()) {
		glm::mat4 mat(1.0);
		mat = glm::translate(mat, data.cube.getMin());
		mat = glm::scale(mat, data.cube.getLength());
		float sdf[8];
		tree->getSdf(data.cube, data.chunkContext, sdf);
		instances->push_back(DebugInstanceData(mat, sdf));
	}
}
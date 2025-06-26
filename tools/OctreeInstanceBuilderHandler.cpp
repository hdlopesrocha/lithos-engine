#include "tools.hpp"


OctreeInstanceBuilderHandler::OctreeInstanceBuilderHandler() {

}

void OctreeInstanceBuilderHandler::handle(OctreeNodeData &data, std::vector<DebugInstanceData> * instances){
	if(data.node->isSolid()) {
		glm::mat4 mat(1.0);
		mat = glm::translate(mat, data.cube.getMin());
		mat = glm::scale(mat, data.cube.getLength());
		instances->push_back(DebugInstanceData(mat, data.node->sdf));
	}
}
#include "tools.hpp"


OctreeInstanceBuilderHandler::OctreeInstanceBuilderHandler() : InstanceBuilderHandler(){

}

void OctreeInstanceBuilderHandler::handle(OctreeNodeData &data, std::vector<InstanceData> * instances){
	//if(data.node->isSolid()) {
		glm::mat4 mat(1.0);
		mat = glm::translate(mat, data.cube.getMin());
		mat = glm::scale(mat, data.cube.getLength());
		instances->push_back(InstanceData(0u, mat , 0.0f));
	//}
}
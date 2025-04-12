#include "tools.hpp"


OctreeInstanceBuilderHandler::OctreeInstanceBuilderHandler(Octree * tree, long * count) : InstanceBuilderHandler(tree, count){

}

void OctreeInstanceBuilderHandler::handle(OctreeNodeData &data, InstanceGeometry * pre){
	if(data.node->simplified && data.node->solid == ContainmentType::Intersects) {
		glm::mat4 mat(1.0);
		mat = glm::translate(mat, data.cube.getMin());
		mat = glm::scale(mat, data.cube.getLength());
		InstanceData instance(0u, mat , 0.0f);
		pre->instances.push_back(instance);
	}
}
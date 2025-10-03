#include "tools.hpp"


OctreeInstanceBuilderHandler::OctreeInstanceBuilderHandler() {

}

void OctreeInstanceBuilderHandler::handle(Octree * tree, OctreeNodeData &data, std::vector<DebugInstanceData> * instances, ThreadContext * context){
	if(data.node != NULL) {
		bool virtualizeSDF = false;
		
		if(virtualizeSDF) {
			for(int i = 0 ; i < 8 ; ++i) {
				BoundingCube c = data.cube.getChild(i);
				float s[8];
				SDF::getChildSDF(data.node->sdf, i, s);
				glm::mat4 mat(1.0);
				mat = glm::translate(mat, c.getMin());
				mat = glm::scale(mat, c.getLength());
				instances->push_back(DebugInstanceData(mat, s));
			}
		} else {
			glm::mat4 mat(1.0);
			mat = glm::translate(mat, data.cube.getMin());
			mat = glm::scale(mat, data.cube.getLength());
			instances->push_back(DebugInstanceData(mat, data.node->sdf));
		}
	}
}
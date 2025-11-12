#include "tools.hpp"


OctreeInstanceBuilderHandler::OctreeInstanceBuilderHandler() {

}

void OctreeInstanceBuilderHandler::handle(Octree * tree, OctreeNodeData &data, std::vector<DebugInstanceData> * instances, ThreadContext * context){
	if(data.node->isSimplified() && !data.node->isSolid() && !data.node->isEmpty()) {
		bool virtualizeSDF = false;
		
		if(virtualizeSDF) {
			for(uint i = 0 ; i < 8 ; ++i) {
				BoundingCube c = data.cube.getChild(i);
				float s[8];
				SDF::getChildSDF(data.node->sdf, i, s);
				glm::mat4 mat = glm::scale(glm::translate(glm::mat4(1.0f), c.getMin()), c.getLength());
				instances->push_back(DebugInstanceData(mat, s, data.node->vertex.brushIndex));
			}
		} else {
			glm::mat4 mat = glm::scale(glm::translate(glm::mat4(1.0f), data.cube.getMin()), data.cube.getLength());
			instances->push_back(DebugInstanceData(mat, data.node->sdf, data.node->vertex.brushIndex));
		}
	}
}
#include "tools.hpp"

OctreeGeometryBuilder::~OctreeGeometryBuilder(){

}

OctreeGeometryBuilder::OctreeGeometryBuilder(InstanceBuilderHandler<DebugInstanceData> * handler) {
    this->geometry = new BoxLineGeometry(BoundingBox(glm::vec3(0), glm::vec3(1)));
    this->handler = handler;
}

InstanceGeometry<DebugInstanceData> * OctreeGeometryBuilder::build(OctreeNodeData &params){
    std::vector<DebugInstanceData> instances;

    InstanceBuilder<DebugInstanceData> instanceBuilder(handler, &instances);
    instanceBuilder.iterateFlatIn(params);
	
    //glm::mat4 mat(1.0);
	//mat = glm::translate(mat, params.cube.getMin());
	//mat = glm::scale(mat, params.cube.getLength());
	
	//instances.push_back(InstanceData(0u, mat , 0.0f));

    //TODO: make instance class customizable
    return new InstanceGeometry<DebugInstanceData>(geometry, instances);
}


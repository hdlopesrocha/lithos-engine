#include "tools.hpp"

OctreeGeometryBuilder::~OctreeGeometryBuilder(){

}

OctreeGeometryBuilder::OctreeGeometryBuilder(long * instancesCount, Octree * tree, InstanceBuilderHandler * handler) {
    this->geometry = new BoxLineGeometry(BoundingBox(glm::vec3(0), glm::vec3(1)));
    this->tree = tree;
    this->handler = new OctreeInstanceBuilderHandler(instancesCount );
    this->instancesCount = instancesCount;
}

InstanceGeometry * OctreeGeometryBuilder::build(OctreeNodeData &params){
    InstanceGeometry * instanceGeometry = new InstanceGeometry(geometry);
    InstanceBuilder instanceBuilder(tree, &instanceGeometry->instances, handler, instanceGeometry);
    instanceBuilder.iterateFlatIn(params);
	
    glm::mat4 mat(1.0);
	mat = glm::translate(mat, params.cube.getMin());
	mat = glm::scale(mat, params.cube.getLength());
	
    
    InstanceData instance(0u, mat , 0.0f);
	instanceGeometry->instances.push_back(instance);
    
    *instancesCount += instanceBuilder.instanceCount;
    return instanceGeometry;
}


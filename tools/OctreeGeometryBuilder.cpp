#include "tools.hpp"

OctreeGeometryBuilder::~OctreeGeometryBuilder(){

}

OctreeGeometryBuilder::OctreeGeometryBuilder(int drawableType,long * instancesCount, Octree * tree, InstanceBuilderHandler * handler, std::unordered_map<long, InstanceGeometry*> * map) : GeometryBuilder(drawableType) {
    this->geometry = new BoxLineGeometry(BoundingBox(glm::vec3(0), glm::vec3(1)));
    this->tree = tree;
    this->map = map;
    this->handler = new OctreeInstanceBuilderHandler(tree, instancesCount );
    this->instancesCount = instancesCount;
}

const void OctreeGeometryBuilder::build(OctreeNodeData &params){
    InstanceGeometry * instanceGeometry = new InstanceGeometry(geometry);
    InstanceBuilder instanceBuilder(tree, &instanceGeometry->instances, handler, instanceGeometry);
    instanceBuilder.iterateFlatIn(params);
	
    /*glm::mat4 mat(1.0);
	mat = glm::translate(mat, params.cube.getMin());
	mat = glm::scale(mat, params.cube.getLength());
	
    
    InstanceData instance(0u, mat , 0.0f);
	instanceGeometry->instances.push_back(instance);
    */
    *instancesCount += instanceBuilder.instanceCount;
    map->try_emplace(params.node->dataId, instanceGeometry);
}


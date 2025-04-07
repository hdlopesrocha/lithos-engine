#include "tools.hpp"
#include <algorithm>
#include <random>


OctreeGeometryBuilder::~OctreeGeometryBuilder(){

}

OctreeGeometryBuilder::OctreeGeometryBuilder(int drawableType,long * count, Octree * tree, InstanceBuilderHandler * handler) : GeometryBuilder(drawableType, count) {
    this->geometry = new BoxGeometry(BoundingBox(glm::vec3(0), glm::vec3(1)));
    this->tree = tree;
    this->handler = new OctreeInstanceBuilderHandler(tree, count );
}




const NodeInfo OctreeGeometryBuilder::build(OctreeNodeData &params){
    InstanceGeometry * instanceGeometry = new InstanceGeometry(geometry);
    InstanceBuilder instanceBuilder(tree, &instanceGeometry->instances, handler, instanceGeometry);

    instanceBuilder.iterateFlatIn(params);


    *count += instanceBuilder.instanceCount;

    return NodeInfo(infoType, NULL, instanceGeometry, false);
}


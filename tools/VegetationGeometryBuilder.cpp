#include "tools.hpp"
#include <algorithm>
#include <random>

// Random number generator
std::random_device rd;
std::mt19937 g(rd());  // Mersenne Twister engine

VegetationGeometryBuilder::~VegetationGeometryBuilder(){

}

VegetationGeometryBuilder::VegetationGeometryBuilder(int drawableType,long * instancesCount, Octree * tree, InstanceBuilderHandler * handler, std::unordered_map<long, InstanceGeometry*> * map) : GeometryBuilder(drawableType) {
    this->geometry = new Vegetation3d(1.0);
    this->tree = tree;
    this->handler = new VegetationInstanceBuilderHandler(tree, instancesCount );
    this->instancesCount = instancesCount;
    this->map = map;
}




const void VegetationGeometryBuilder::build(OctreeNodeData &params){
    //std::cout << "VegetationGeometryBuilder::build" <<std::endl;
    InstanceGeometry * instanceGeometry = new InstanceGeometry(geometry);
    InstanceBuilder instanceBuilder(tree, &instanceGeometry->instances, handler, instanceGeometry);

    instanceBuilder.iterateFlatIn(params);

    // Shuffle the vector
    if(instanceGeometry->instances.size()){
        std::shuffle(instanceGeometry->instances.begin(), instanceGeometry->instances.end(), g);
    }
    *instancesCount += instanceBuilder.instanceCount;

    map->try_emplace(params.node->dataId, instanceGeometry);
}


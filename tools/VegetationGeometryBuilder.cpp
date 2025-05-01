#include "tools.hpp"
#include <algorithm>
#include <random>

// Random number generator
std::random_device rd;
std::mt19937 g(rd());  // Mersenne Twister engine

VegetationGeometryBuilder::~VegetationGeometryBuilder(){

}

VegetationGeometryBuilder::VegetationGeometryBuilder(long * instancesCount, Octree * tree, InstanceBuilderHandler * handler) {
    this->geometry = new Vegetation3d(1.0);
    this->tree = tree;
    this->handler = handler;
    this->instancesCount = instancesCount;
}

InstanceGeometry * VegetationGeometryBuilder::build(OctreeNodeData &params){
    //std::cout << "VegetationGeometryBuilder::build" <<std::endl;
    InstanceGeometry * instanceGeometry = new InstanceGeometry(geometry);
    InstanceBuilder instanceBuilder(handler, instanceGeometry);

    instanceBuilder.iterateFlatIn(params);

    // Shuffle the vector
    if(instanceGeometry->instances.size()) {
        std::shuffle(instanceGeometry->instances.begin(), instanceGeometry->instances.end(), g);
    }
    *instancesCount += instanceGeometry->instances.size();
    return instanceGeometry;
}


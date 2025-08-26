#include "tools.hpp"
#include <algorithm>
#include <random>

// Random number generator
std::random_device rd;
std::mt19937 g(rd());  // Mersenne Twister engine

VegetationGeometryBuilder::~VegetationGeometryBuilder(){

}

VegetationGeometryBuilder::VegetationGeometryBuilder(InstanceBuilderHandler<InstanceData> * handler) {
    this->geometry = new Vegetation3d(1.0);
    this->handler = handler;
}

InstanceGeometry<InstanceData> * VegetationGeometryBuilder::build(Octree * tree, OctreeNodeData &params){
    //std::cout << "VegetationGeometryBuilder::build" <<std::endl;
    
    std::vector<InstanceData> instances;
    InstanceBuilder instanceBuilder(handler, &instances);

    instanceBuilder.iterateFlatIn(tree, params);

    // Shuffle the vector
    if(instances.size()) {
        std::shuffle(instances.begin(), instances.end(), g);
        return new InstanceGeometry<InstanceData>(geometry, instances);
    }
    else {
        return NULL;
    }
}


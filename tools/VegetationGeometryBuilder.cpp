#include "tools.hpp"
#include <algorithm>
#include <random>

// Random number generator
std::random_device rd;
std::mt19937 g(rd());  // Mersenne Twister engine

VegetationGeometryBuilder::~VegetationGeometryBuilder(){

}

VegetationGeometryBuilder::VegetationGeometryBuilder(int drawableType,long * count, Octree * tree, InstanceBuilderHandler * handler) : GeometryBuilder(drawableType, count) {
    this->geometry = new Vegetation3d(1.0);
    this->tree = tree;
    this->handler = new VegetationInstanceBuilderHandler(tree, count );
}




const NodeInfo VegetationGeometryBuilder::build(int level, int height, int lod, OctreeNode* node, BoundingCube cube){
    //std::cout << "VegetationGeometryBuilder::build" <<std::endl;
    InstanceGeometry * instanceGeometry = new InstanceGeometry(geometry);
    InstanceBuilder instanceBuilder(tree, &instanceGeometry->instances, handler);
    instanceBuilder.iterateFlatIn(level, height, lod, node, cube, instanceGeometry);

    // Shuffle the vector
    if(instanceGeometry->instances.size()){
        std::shuffle(instanceGeometry->instances.begin(), instanceGeometry->instances.end(), g);
    }
    *count += instanceBuilder.instanceCount;

    return NodeInfo(infoType, NULL, instanceGeometry, false);
}


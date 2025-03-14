#include "gl.hpp"
#include <algorithm>
#include <random>

// Random number generator
std::random_device rd;
std::mt19937 g(rd());  // Mersenne Twister engine

VegetationGeometryBuilder::~VegetationGeometryBuilder(){

}

VegetationGeometryBuilder::VegetationGeometryBuilder(int drawableType,long * count, Octree * tree) : GeometryBuilder(drawableType, count) {
    this->geometry = new Vegetation3d(1.0);
    this->tree = tree;
}




const NodeInfo VegetationGeometryBuilder::build(int level, int height, int lod, OctreeNode* node, BoundingCube cube){
    //std::cout << "VegetationGeometryBuilder::build" <<std::endl;

    PreLoadedGeometry * pre = new PreLoadedGeometry(geometry);
    InstanceBuilder instanceBuilder(tree, &pre->instances);
    instanceBuilder.iterateFlatIn(level, height, lod, node, cube, NULL);

    // Shuffle the vector
    if(pre->instances.size()){
        std::shuffle(pre->instances.begin(), pre->instances.end(), g);
    }
    *count += instanceBuilder.instanceCount;

    return NodeInfo(drawableType, NULL, pre, false);
}


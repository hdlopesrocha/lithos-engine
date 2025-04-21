#include "space.hpp"


MeshGeometryBuilder::~MeshGeometryBuilder(){
    
}

MeshGeometryBuilder::MeshGeometryBuilder(long * instancesCount, long * trianglesCount, Octree * tree, float simplificationAngle, float simplificationDistance, bool simplificationTexturing) {
    this->tree = tree;
	this->simplificationAngle = simplificationAngle;
	this->simplificationDistance = simplificationDistance;
	this->simplificationTexturing = simplificationTexturing;
    this->instancesCount = instancesCount;
    this->trianglesCount = trianglesCount;
}

InstanceGeometry * MeshGeometryBuilder::build(OctreeNodeData &params){
    //std::cout << "MeshGeometryBuilder::build" <<std::endl;
    // Simplify
    Simplifier simplifier(tree, params.cube, simplificationAngle, simplificationDistance, simplificationTexturing); 
    simplifier.iterateFlatOut(params);

    // Tesselate
    Geometry * geometry = new Geometry(false);

    Tesselator tesselator(tree, geometry, trianglesCount);
    tesselator.iterateFlatIn(params);

    InstanceGeometry * pre = new InstanceGeometry(geometry);
    pre->instances.push_back(InstanceData(0, glm::mat4(1.0), 0.0f));

    *instancesCount += 1;
    return pre;
}
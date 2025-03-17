#include "math.hpp"


MeshGeometryBuilder::~MeshGeometryBuilder(){
    
}

MeshGeometryBuilder::MeshGeometryBuilder(int drawableType, long * count, Octree * tree,float simplificationAngle, float simplificationDistance, bool simplificationTexturing) : GeometryBuilder(drawableType, count){
    this->tree = tree;
	this->simplificationAngle = simplificationAngle;
	this->simplificationDistance = simplificationDistance;
	this->simplificationTexturing = simplificationTexturing;
}

const NodeInfo MeshGeometryBuilder::build(OctreeNodeData &params){
    //std::cout << "MeshGeometryBuilder::build" <<std::endl;
    // Simplify
    Simplifier simplifier(tree, params.cube, simplificationAngle, simplificationDistance, simplificationTexturing); 
    simplifier.iterateFlatOut(params);

    // Tesselate
    Geometry * geometry = new Geometry();

    Tesselator tesselator(tree, geometry, count);
    tesselator.iterateFlatIn(params);

    InstanceGeometry * pre = new InstanceGeometry(geometry);
    pre->instances.push_back(InstanceData(0, glm::mat4(1.0), 0.0f));

    *count += 1;

    return NodeInfo(infoType, NULL, pre, false);
}
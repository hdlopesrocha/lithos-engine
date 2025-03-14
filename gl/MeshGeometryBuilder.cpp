#include "gl.hpp"


MeshGeometryBuilder::~MeshGeometryBuilder(){
    
}
MeshGeometryBuilder::MeshGeometryBuilder(int drawableType, long * count, Octree * tree,float simplificationAngle, float simplificationDistance, bool simplificationTexturing, int simplification) : GeometryBuilder(drawableType, count){
    this->tree = tree;
	this->simplificationAngle = simplificationAngle;
	this->simplificationDistance = simplificationDistance;
	this->simplificationTexturing = simplificationTexturing;
    this->simplification = simplification;
}

const NodeInfo MeshGeometryBuilder::build(int level, int height, int lod, OctreeNode* node, BoundingCube cube){
    //std::cout << "MeshGeometryBuilder::build" <<std::endl;
    // Simplify
    Simplifier simplifier(tree, cube, simplificationAngle, simplificationDistance, simplificationTexturing, simplification); 
    simplifier.iterateFlatOut(level, height, lod, node, cube, NULL);

    // Tesselate
    Geometry * geometry = new Geometry();
    Tesselator tesselator(tree, geometry, simplification);
    tesselator.iterateFlatIn(level, height, lod, node, cube, NULL);

    PreLoadedGeometry * pre = new PreLoadedGeometry(geometry);
    pre->instances.push_back(InstanceData(glm::mat4(1.0), 0.0f));

    *count += 1;

    return NodeInfo(drawableType, NULL, pre, false);
}
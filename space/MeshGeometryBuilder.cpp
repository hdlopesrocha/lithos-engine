#include "space.hpp"


MeshGeometryBuilder::~MeshGeometryBuilder(){
    
}

MeshGeometryBuilder::MeshGeometryBuilder(long * trianglesCount, Octree * tree) {
    this->tree = tree;
    this->trianglesCount = trianglesCount;
}

InstanceGeometry * MeshGeometryBuilder::build(OctreeNodeData &params){

    // Tesselate
    Geometry * geometry = new Geometry(false);

    Tesselator tesselator(tree, geometry, trianglesCount);
    tesselator.iterateFlatIn(params);
    if(geometry->indices.size() > 0) {
        InstanceGeometry * pre = new InstanceGeometry(geometry);
        pre->instances.push_back(InstanceData(0, glm::mat4(1.0), 0.0f));
        return pre;
    } else {
        delete geometry;
        return NULL;
    }
}
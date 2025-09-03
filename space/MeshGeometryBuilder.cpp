#include "space.hpp"


MeshGeometryBuilder::~MeshGeometryBuilder(){
    
}

MeshGeometryBuilder::MeshGeometryBuilder(long * trianglesCount) {
    this->trianglesCount = trianglesCount;
}

InstanceGeometry<InstanceData>* MeshGeometryBuilder::build(Octree * tree, OctreeNodeData &params, ChunkContext * context){

    // Tesselate
    Geometry * geometry = new Geometry(false);

    Tesselator tesselator(geometry, trianglesCount, context);
    tesselator.iterateFlatIn(tree, params);
    if(geometry->indices.size() > 0) {
        InstanceGeometry<InstanceData> * pre = new InstanceGeometry<InstanceData>(geometry);
        pre->instances.push_back(InstanceData(0, glm::mat4(1.0), 0.0f));
        return pre;
    } else {
        delete geometry;
        return NULL;
    }
}
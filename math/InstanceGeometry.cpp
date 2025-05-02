#include "math.hpp"

InstanceGeometry::InstanceGeometry(Geometry * geometry) {
    this->geometry = geometry;
    geometry->setCenter();
}

InstanceGeometry::InstanceGeometry(Geometry * geometry, std::vector<InstanceData> &instances) {
    this->geometry = geometry;
    this->instances = instances;
    geometry->setCenter();
}



InstanceGeometry::~InstanceGeometry() {
    if(!geometry->reusable){
        delete geometry;
    }
}
#include "math.hpp"

InstanceGeometry::InstanceGeometry(Geometry * geometry) {
    this->geometry = geometry;
    geometry->setCenter();
}

InstanceGeometry::~InstanceGeometry() {
    if(!geometry->reusable){
        delete geometry;
    }
}
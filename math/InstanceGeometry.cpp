#include "math.hpp"

InstanceGeometry::InstanceGeometry(Geometry * geometry) {
    this->geometry = geometry;
    geometry->setCenter();
}
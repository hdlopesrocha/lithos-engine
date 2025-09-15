#include "SDF.hpp"


OctahedronDistanceFunction::OctahedronDistanceFunction() {
    
}

float OctahedronDistanceFunction::distance(const glm::vec3 p, const Transformation &model) {
    glm::vec3 pos = p - getCenter(model);
    pos = glm::inverse(model.quaternion) * pos;

    glm::vec3 q = pos / model.scale;

    float d = SDF::octahedron(q, 1.0f);
    float minScale = glm::min(glm::min(model.scale.x, model.scale.y), model.scale.z);

    return d * minScale;
}

SdfType OctahedronDistanceFunction::getType() const {
    return SdfType::OCTAHEDRON;
}

glm::vec3 OctahedronDistanceFunction::getCenter(const Transformation &model) const {
    return model.translate;
}


WrappedOctahedron::WrappedOctahedron(OctahedronDistanceFunction * function) : WrappedSignedDistanceFunction(function) {

}

WrappedOctahedron::~WrappedOctahedron() {

}

BoundingSphere WrappedOctahedron::getSphere(const Transformation &model, float bias) const {
    OctahedronDistanceFunction * f = (OctahedronDistanceFunction*) function;
    return BoundingSphere(f->getCenter(model), glm::length(model.scale) + bias);
};

ContainmentType WrappedOctahedron::check(const BoundingCube &cube, const Transformation &model, float bias) const {
    BoundingSphere sphere = getSphere(model, bias);
    return sphere.test(cube);
};

bool WrappedOctahedron::isContained(const BoundingCube &cube, const Transformation &model, float bias) const {
    BoundingSphere sphere = getSphere(model, bias);
    return cube.contains(sphere);
};

float WrappedOctahedron::getLength(const Transformation &model, float bias) const {
    return glm::length(model.scale) + bias;
};

void WrappedOctahedron::accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const {
    getSphere(model, bias).accept(visitor);
}    

const char* WrappedOctahedron::getLabel() const {
    return "Octahedron";
}
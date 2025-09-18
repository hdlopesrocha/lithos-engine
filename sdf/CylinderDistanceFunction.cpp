#include "SDF.hpp"


CylinderDistanceFunction::CylinderDistanceFunction() {
    
}

float CylinderDistanceFunction::distance(const glm::vec3 p, const Transformation &model) {
 // Transform point into local space
    glm::vec3 pos = p - getCenter(model);
    pos = glm::inverse(model.quaternion) * pos;
    glm::vec3 q = pos / model.scale;

    // Distance in local space
    float d = SDF::cylinder(q, 0.5f, 1.0f);

    // Rescale back to world space
    float minScale = glm::min(glm::min(model.scale.x, model.scale.y), model.scale.z);
    return d * minScale;
}

SdfType CylinderDistanceFunction::getType() const {
    return SdfType::CYLINDER;
}

glm::vec3 CylinderDistanceFunction::getCenter(const Transformation &model) const {
    return model.translate;
}


WrappedCylinder::WrappedCylinder(CylinderDistanceFunction * function) : WrappedSignedDistanceFunction(function) {

}

WrappedCylinder::~WrappedCylinder() {

}

BoundingSphere WrappedCylinder::getSphere(const Transformation &model, float bias) const {
    CylinderDistanceFunction * f = (CylinderDistanceFunction*) function;
    return BoundingSphere(f->getCenter(model), glm::length(model.scale)*sqrt(0.5f) + bias);
};

ContainmentType WrappedCylinder::check(const BoundingCube &cube, const Transformation &model, float bias) const {
    BoundingSphere sphere = getSphere(model, bias);
    return sphere.test(cube);
};

bool WrappedCylinder::isContained(const BoundingCube &cube, const Transformation &model, float bias) const {
    BoundingSphere sphere = getSphere(model, bias);
    return cube.contains(sphere);
};

float WrappedCylinder::getLength(const Transformation &model, float bias) const {
    return glm::length(model.scale) + bias;
};

void WrappedCylinder::accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const {
    getSphere(model, bias).accept(visitor);
}

const char* WrappedCylinder::getLabel() const {
    return "Cylinder";
}
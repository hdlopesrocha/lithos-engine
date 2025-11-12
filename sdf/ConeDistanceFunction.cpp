#include "SDF.hpp"


ConeDistanceFunction::ConeDistanceFunction() {
    
}

float ConeDistanceFunction::distance(const glm::vec3 &p, const Transformation &model) {
 // Transform point into local space
    glm::vec3 pos = p - getCenter(model);
    pos = glm::inverse(model.quaternion) * pos;
    glm::vec3 q = pos / model.scale - glm::vec3(0,1,0);

    // Distance in local space
    float d = SDF::cone(q);

    // Rescale back to world space
    float minScale = glm::min(glm::min(model.scale.x, model.scale.y), model.scale.z);
    return d * minScale;
}

SdfType ConeDistanceFunction::getType() const {
    return SdfType::CONE;
}

glm::vec3 ConeDistanceFunction::getCenter(const Transformation &model) const {
    return model.translate;
}


WrappedCone::WrappedCone(ConeDistanceFunction * function) : WrappedSignedDistanceFunction(function) {

}

WrappedCone::~WrappedCone() {

}

BoundingSphere WrappedCone::getSphere(const Transformation &model, float bias) const {
    ConeDistanceFunction * f = (ConeDistanceFunction*) function;
    return BoundingSphere(f->getCenter(model), sqrt(0.5f) * glm::length(model.scale) + bias);
};

ContainmentType WrappedCone::check(const BoundingCube &cube, const Transformation &model, float bias) const {
    BoundingSphere sphere = getSphere(model, bias);
    return sphere.test(cube);
};

bool WrappedCone::isContained(const BoundingCube &cube, const Transformation &model, float bias) const {
    BoundingSphere sphere = getSphere(model, bias);
    return cube.contains(sphere);
};

float WrappedCone::getLength(const Transformation &model, float bias) const {
    return glm::length(model.scale) + bias;
};

void WrappedCone::accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const {
    getSphere(model, bias).accept(visitor);
}

const char* WrappedCone::getLabel() const {
    return "Cone";
}
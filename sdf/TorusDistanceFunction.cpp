#include "SDF.hpp"

TorusDistanceFunction::TorusDistanceFunction(glm::vec2 radius): radius(radius) {
    
}

float TorusDistanceFunction::distance(const glm::vec3 p, const Transformation &model) {
     glm::vec3 pos = p - getCenter(model);
    pos = glm::inverse(model.quaternion) * pos;

    glm::vec3 q = pos / model.scale;
    float d = SDF::torus(q, radius);

    float minScale = glm::min(glm::min(model.scale.x, model.scale.y), model.scale.z);
    return d * minScale;
}

SdfType TorusDistanceFunction::getType() const {
    return SdfType::TORUS;
}

glm::vec3 TorusDistanceFunction::getCenter(const Transformation &model) const {
    return model.translate;
}

WrappedTorus::WrappedTorus(TorusDistanceFunction * function) : WrappedSignedDistanceFunction(function) {

}

WrappedTorus::~WrappedTorus() {

}

BoundingSphere WrappedTorus::getSphere(const Transformation &model, float bias) const {
    TorusDistanceFunction * f = (TorusDistanceFunction*) function;
    return BoundingSphere(f->getCenter(model), glm::length(model.scale)*sqrt(0.5f) + bias);
};

ContainmentType WrappedTorus::check(const BoundingCube &cube, const Transformation &model, float bias) const {
    BoundingSphere sphere = getSphere(model, bias);
    return sphere.test(cube);
};

bool WrappedTorus::isContained(const BoundingCube &cube, const Transformation &model, float bias) const {
    BoundingSphere sphere = getSphere(model, bias);
    return cube.contains(sphere);
};

float WrappedTorus::getLength(const Transformation &model, float bias) const {
    return glm::length(model.scale) + bias;
};

void WrappedTorus::accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const {
    getSphere(model, bias).accept(visitor);
}

const char* WrappedTorus::getLabel() const {
    return "Torus";
}
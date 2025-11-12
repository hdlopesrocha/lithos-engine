#include "SDF.hpp"

CapsuleDistanceFunction::CapsuleDistanceFunction(glm::vec3 a, glm::vec3 b, float r) {
    this->a = a;
    this->b = b;
    this->radius = r;
}

float CapsuleDistanceFunction::distance(const glm::vec3 &p, const Transformation &model) {
    glm::vec3 pos = p - model.translate; // Move point into model space
    pos = glm::inverse(model.quaternion) * pos;
    return SDF::capsule(pos/model.scale, a, b, radius);
}

SdfType CapsuleDistanceFunction::getType() const {
    return SdfType::CAPSULE;
}

glm::vec3 CapsuleDistanceFunction::getCenter(const Transformation &model) const {
    return 0.5f*(this->a+this->b)+model.translate;
}

WrappedCapsule::WrappedCapsule(CapsuleDistanceFunction * function) : WrappedSignedDistanceFunction(function) {

}

WrappedCapsule::~WrappedCapsule() {

}

BoundingBox WrappedCapsule::getBox(const Transformation &model, float bias) const {
    CapsuleDistanceFunction * f = (CapsuleDistanceFunction*) function;
    glm::vec3 min = glm::min(f->a, f->b)*glm::length(model.scale)+model.translate;
    glm::vec3 max = glm::max(f->a, f->b)*glm::length(model.scale)+model.translate;
    glm::vec3 len = glm::vec3(f->radius  + bias);


    return BoundingBox(min - len, max + len);
};

ContainmentType WrappedCapsule::check(const BoundingCube &cube, const Transformation &model, float bias) const {
    BoundingBox box = getBox(model, bias);
    return box.test(cube);
};

bool WrappedCapsule::isContained(const BoundingCube &cube, const Transformation &model, float bias) const {
    BoundingBox box = getBox(model, bias);
    return cube.contains(box);
};
float WrappedCapsule::getLength(const Transformation &model, float bias) const {
    BoundingBox box = getBox(model, bias);
    return glm::distance(box.getMin(), box.getMax());
};

void WrappedCapsule::accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const {
    getBox(model, bias).accept(visitor);
}

const char* WrappedCapsule::getLabel() const {
    return "Capsule";
}
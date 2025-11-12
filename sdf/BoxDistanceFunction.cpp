#include "SDF.hpp"

BoxDistanceFunction::BoxDistanceFunction() {

}

float BoxDistanceFunction::distance(const glm::vec3 &p, const Transformation &model) {
    glm::vec3 pos = p - getCenter(model); // Move point into model space
    pos = glm::inverse(model.quaternion) * pos;
    return SDF::box(pos, model.scale);
}

SdfType BoxDistanceFunction::getType() const {
    return SdfType::BOX;
}

glm::vec3 BoxDistanceFunction::getCenter(const Transformation &model) const {
    return model.translate;
}


WrappedBox::WrappedBox(BoxDistanceFunction * function) : WrappedSignedDistanceFunction(function) {

}

WrappedBox::~WrappedBox() {

}

BoundingSphere WrappedBox::getSphere(const Transformation &model, float bias) const {
    BoxDistanceFunction * f = (BoxDistanceFunction*) function;
    return BoundingSphere(f->getCenter(model), glm::length(model.scale)+ bias);
};

ContainmentType WrappedBox::check(const BoundingCube &cube, const Transformation &model, float bias) const {
    BoundingSphere sphere = getSphere(model, bias);
    return sphere.test(cube);
};

bool WrappedBox::isContained(const BoundingCube &cube, const Transformation &model, float bias) const {
    BoundingSphere sphere = getSphere(model, bias);
    return cube.contains(sphere);
};

float WrappedBox::getLength(const Transformation &model, float bias) const {
    return glm::length(model.scale) + bias;
};

void WrappedBox::accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const {
    getSphere(model, bias).accept(visitor);
}

const char* WrappedBox::getLabel() const {
    return "Box";
}
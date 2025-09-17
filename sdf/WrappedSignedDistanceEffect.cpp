#include "SDF.hpp"

WrappedSignedDistanceEffect::WrappedSignedDistanceEffect(WrappedSignedDistanceFunction * function) : WrappedSignedDistanceFunction(function) {

}

WrappedSignedDistanceEffect::~WrappedSignedDistanceEffect() {

}

void WrappedSignedDistanceEffect::setFunction(WrappedSignedDistanceFunction * function) {
    this->function = function;
}

ContainmentType WrappedSignedDistanceEffect::check(const BoundingCube &cube, const Transformation &model, float bias) const {
    WrappedSignedDistanceFunction * f = (WrappedSignedDistanceFunction*) function;
    return f->check(cube, model, bias);
};

bool WrappedSignedDistanceEffect::isContained(const BoundingCube &cube, const Transformation &model, float bias) const {
    WrappedSignedDistanceFunction * f = (WrappedSignedDistanceFunction*) function;
    return f->isContained(cube, model, bias);
};

float WrappedSignedDistanceEffect::getLength(const Transformation &model, float bias) const {
    WrappedSignedDistanceFunction * f = (WrappedSignedDistanceFunction*) function;
    return f->getLength(model, bias);
};

void WrappedSignedDistanceEffect::accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const {
    WrappedSignedDistanceFunction * f = (WrappedSignedDistanceFunction*) function;
    f->accept(visitor, model, bias);
}


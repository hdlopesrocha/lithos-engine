#include "SDF.hpp"

WrappedPerlinDistortDistanceFunction::WrappedPerlinDistortDistanceFunction(WrappedSignedDistanceFunction * function) : WrappedSignedDistanceFunction(function) {

}

WrappedPerlinDistortDistanceFunction::~WrappedPerlinDistortDistanceFunction() {

}

ContainmentType WrappedPerlinDistortDistanceFunction::check(const BoundingCube &cube, const Transformation &model, float bias) const {
    WrappedSignedDistanceFunction * f = (WrappedSignedDistanceFunction*) function;
    return f->check(cube, model, bias);
};

bool WrappedPerlinDistortDistanceFunction::isContained(const BoundingCube &cube, const Transformation &model, float bias) const {
    WrappedSignedDistanceFunction * f = (WrappedSignedDistanceFunction*) function;
    return f->isContained(cube, model, bias);
};
float WrappedPerlinDistortDistanceFunction::getLength(const Transformation &model, float bias) const {
    WrappedSignedDistanceFunction * f = (WrappedSignedDistanceFunction*) function;
    return f->getLength(model, bias);
};

void WrappedPerlinDistortDistanceFunction::accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const {
    WrappedSignedDistanceFunction * f = (WrappedSignedDistanceFunction*) function;
    f->accept(visitor, model, bias);
}    

const char* WrappedPerlinDistortDistanceFunction::getLabel() const {
    return "Perlin Distort";
}

float WrappedPerlinDistortDistanceFunction::distance(const glm::vec3 p, const Transformation &model) {
    glm::vec3 newPos = SDF::distortPerlinFractal(p, 48.0f, 0.1f/32.0f, 6, 2.0f, 0.5f);
    return function->distance(newPos, model);
}

#include "SDF.hpp"

WrappedPerlinCarveDistanceFunction::WrappedPerlinCarveDistanceFunction(WrappedSignedDistanceFunction * function) : WrappedSignedDistanceFunction(function) {

}

WrappedPerlinCarveDistanceFunction::~WrappedPerlinCarveDistanceFunction() {

}

ContainmentType WrappedPerlinCarveDistanceFunction::check(const BoundingCube &cube, const Transformation &model, float bias) const {
    WrappedSignedDistanceFunction * f = (WrappedSignedDistanceFunction*) function;
    return f->check(cube, model, bias);
};

bool WrappedPerlinCarveDistanceFunction::isContained(const BoundingCube &cube, const Transformation &model, float bias) const {
    WrappedSignedDistanceFunction * f = (WrappedSignedDistanceFunction*) function;
    return f->isContained(cube, model, bias);
};
float WrappedPerlinCarveDistanceFunction::getLength(const Transformation &model, float bias) const {
    WrappedSignedDistanceFunction * f = (WrappedSignedDistanceFunction*) function;
    return f->getLength(model, bias);
};

void WrappedPerlinCarveDistanceFunction::accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const {
    WrappedSignedDistanceFunction * f = (WrappedSignedDistanceFunction*) function;
    f->accept(visitor, model, bias);
}    

const char* WrappedPerlinCarveDistanceFunction::getLabel() const {
    return "Perlin Carve";
}

float WrappedPerlinCarveDistanceFunction::distance(const glm::vec3 p, const Transformation &model) {
    float d = function->distance(p, model);
    return SDF::distortedCarveFractalSDF(p, d, 0.0f, 64.0f, 0.1f/32.0f, 6, 2.0f, 0.5f);
}

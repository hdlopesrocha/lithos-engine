#include "SDF.hpp"


PyramidDistanceFunction::PyramidDistanceFunction() {
    
}

float PyramidDistanceFunction::distance(const glm::vec3 p, const Transformation &model)  {
   glm::vec3 pos = p - getCenter(model);
    pos = glm::inverse(model.quaternion) * pos;

    // aplicar escala ao ponto, não à geometria
    pos /= model.scale;

    // pirâmide unitária (base half=0.5, altura=1.0)
    float d = SDF::pyramid(pos, 1.0f, sqrt(0.5f));

    // corrigir métrica multiplicando pela menor escala
    float minScale = glm::min(glm::min(model.scale.x, model.scale.y), model.scale.z);
    return d * minScale;
}

SdfType PyramidDistanceFunction::getType() const {
    return SdfType::PYRAMID;
}

glm::vec3 PyramidDistanceFunction::getCenter(const Transformation &model) const {
    return model.translate;
}

WrappedPyramid::WrappedPyramid(PyramidDistanceFunction * function) : WrappedSignedDistanceFunction(function) {

}

WrappedPyramid::~WrappedPyramid() {

}

float WrappedPyramid::boundingSphereRadius(float width, float depth, float height) const {
    return glm::length(glm::vec3(width, height, depth));
}

BoundingSphere WrappedPyramid::getSphere(const Transformation &model, float bias) const {
    PyramidDistanceFunction * f = (PyramidDistanceFunction*) function;
    return BoundingSphere(f->getCenter(model), sqrt(0.5f) * glm::length(model.scale) + bias);
};

ContainmentType WrappedPyramid::check(const BoundingCube &cube, const Transformation &model, float bias) const {
    BoundingSphere sphere = getSphere(model, bias);
    return sphere.test(cube);
};

bool WrappedPyramid::isContained(const BoundingCube &cube, const Transformation &model, float bias) const {
    BoundingSphere sphere = getSphere(model, bias);
    return cube.contains(sphere);
};
float WrappedPyramid::getLength(const Transformation &model, float bias) const {
    return glm::length(model.scale) + bias;
};

void WrappedPyramid::accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const {
    getSphere(model, bias).accept(visitor);
}    

const char* WrappedPyramid::getLabel() const {
    return "Pyramid";
}
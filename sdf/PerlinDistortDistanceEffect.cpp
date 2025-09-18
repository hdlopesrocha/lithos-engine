#include "SDF.hpp"

WrappedPerlinDistortDistanceEffect::WrappedPerlinDistortDistanceEffect(WrappedSignedDistanceFunction * function, float amplitude, float frequency, glm::vec3 offset) : WrappedSignedDistanceEffect(NULL), amplitude(amplitude), frequency(frequency), offset(offset) {
    this->setFunction(function);
}

WrappedPerlinDistortDistanceEffect::~WrappedPerlinDistortDistanceEffect() {

}

const char* WrappedPerlinDistortDistanceEffect::getLabel() const {
    return "Perlin Distort";
}

float WrappedPerlinDistortDistanceEffect::distance(const glm::vec3 p, const Transformation &model) {
    glm::vec3 newPos = SDF::distortPerlinFractal(p, amplitude, frequency, offset, 6, 2.0f, 0.5f);
    return function->distance(newPos, model);
}

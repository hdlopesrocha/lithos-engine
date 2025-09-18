#include "SDF.hpp"

WrappedSineDistortDistanceEffect::WrappedSineDistortDistanceEffect(WrappedSignedDistanceFunction * function, float amplitude, float frequency, glm::vec3 offset) : WrappedSignedDistanceEffect(NULL), amplitude(amplitude), frequency(frequency), offset(offset) {
    this->setFunction(function);
}

WrappedSineDistortDistanceEffect::~WrappedSineDistortDistanceEffect() {

}

const char* WrappedSineDistortDistanceEffect::getLabel() const {
    return "Sine Distort";
}

float WrappedSineDistortDistanceEffect::distance(const glm::vec3 p, const Transformation &model) {
    float dx = sin((p.y + offset.x) * frequency) * amplitude;
    float dy = sin((p.z + offset.y) * frequency) * amplitude;
    float dz = sin((p.x + offset.z) * frequency) * amplitude; 
    glm::vec3 newPos = p + glm::vec3(dx, dy, dz);

    return function->distance(newPos, model);
}

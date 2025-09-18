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
    glm::vec3 pp = p + offset; // apply offset

    float dx = sin(pp.x * frequency) * cos(pp.y * frequency) * sin(pp.z * frequency);
    float dy = cos(pp.x * frequency) * sin(pp.y * frequency) * cos(pp.z * frequency);
    float dz = sin(pp.x * frequency) * sin(pp.y * frequency) * cos(pp.z * frequency);

    const float norm = 2.0f; // sum of two [-1,1] waves
    glm::vec3 newPos = p + amplitude / norm * glm::vec3(dx, dy, dz);

    return function->distance(newPos, model);
}

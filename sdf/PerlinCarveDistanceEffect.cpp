#include "SDF.hpp"

WrappedPerlinCarveDistanceEffect::WrappedPerlinCarveDistanceEffect(WrappedSignedDistanceFunction * function, float amplitude, float frequency, float threshold, glm::vec3 offset) : WrappedSignedDistanceEffect(function), amplitude(amplitude), frequency(frequency), threshold(threshold), offset(offset) {

}

WrappedPerlinCarveDistanceEffect::~WrappedPerlinCarveDistanceEffect() {

}

const char* WrappedPerlinCarveDistanceEffect::getLabel() const {
    return "Perlin Carve";
}

float WrappedPerlinCarveDistanceEffect::distance(const glm::vec3 p, const Transformation &model) {
    float d = function->distance(p, model);
    return d + SDF::distortedCarveFractalSDF(p+offset, threshold, amplitude, frequency, 6, 2.0f, 0.5f);
}

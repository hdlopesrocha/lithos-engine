#include "SDF.hpp"

WrappedPerlinCarveDistanceEffect::WrappedPerlinCarveDistanceEffect(WrappedSignedDistanceFunction * function, float amplitude, float frequency, float threshold) : WrappedSignedDistanceEffect(function), amplitude(amplitude), frequency(frequency), threshold(threshold) {

}

WrappedPerlinCarveDistanceEffect::~WrappedPerlinCarveDistanceEffect() {

}

const char* WrappedPerlinCarveDistanceEffect::getLabel() const {
    return "Perlin Carve";
}

float WrappedPerlinCarveDistanceEffect::distance(const glm::vec3 p, const Transformation &model) {
    float d = function->distance(p, model);
    return SDF::distortedCarveFractalSDF(p, d, threshold, amplitude, frequency, 6, 2.0f, 0.5f);
}

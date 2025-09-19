#include "SDF.hpp"

WrappedPerlinCarveDistanceEffect::WrappedPerlinCarveDistanceEffect(WrappedSignedDistanceFunction * function, float amplitude, float frequency, float threshold, glm::vec3 offset, float brightness, float contrast) : WrappedSignedDistanceEffect(function), amplitude(amplitude), frequency(frequency), threshold(threshold), offset(offset), brightness(brightness), contrast(contrast) {

}

WrappedPerlinCarveDistanceEffect::~WrappedPerlinCarveDistanceEffect() {

}

const char* WrappedPerlinCarveDistanceEffect::getLabel() const {
    return "Perlin Carve";
}

float WrappedPerlinCarveDistanceEffect::distance(const glm::vec3 p, const Transformation &model) {
    float d = function->distance(p, model);
    float noise = SDF::distortedCarveFractalSDF(p+offset, threshold, frequency, 6, 2.0f, 0.5f);
    noise = Math::brightnessAndContrast(noise, brightness, contrast);

    return d + noise * amplitude;
}

ContainmentType WrappedPerlinCarveDistanceEffect::check(const BoundingCube &cube, const Transformation &model, float bias) const {
    return WrappedSignedDistanceEffect::check(cube, model, bias+amplitude);
};
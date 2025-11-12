#include "SDF.hpp"

WrappedPerlinDistortDistanceEffect::WrappedPerlinDistortDistanceEffect(WrappedSignedDistanceFunction * function, float amplitude, float frequency, glm::vec3 offset, float brightness, float contrast) : WrappedSignedDistanceEffect(NULL), amplitude(amplitude), frequency(frequency), offset(offset), brightness(brightness), contrast(contrast) {
    this->setFunction(function);
}

WrappedPerlinDistortDistanceEffect::~WrappedPerlinDistortDistanceEffect() {

}

const char* WrappedPerlinDistortDistanceEffect::getLabel() const {
    return "Perlin Distort";
}

float WrappedPerlinDistortDistanceEffect::distance(const glm::vec3 &p, const Transformation &model) {
    glm::vec3 noise = SDF::distortPerlinFractal(p+offset, frequency, 6, 2.0f, 0.5f);
    noise.x = Math::brightnessAndContrast(noise.x, brightness, contrast);
    noise.y = Math::brightnessAndContrast(noise.y, brightness, contrast);
    noise.z = Math::brightnessAndContrast(noise.z, brightness, contrast);

    glm::vec3 newPos = p + amplitude * noise;
    float d = function->distance(newPos, model);
    return d;
}

ContainmentType WrappedPerlinDistortDistanceEffect::check(const BoundingCube &cube, const Transformation &model, float bias) const {
    return WrappedSignedDistanceEffect::check(cube, model, bias+amplitude);
};
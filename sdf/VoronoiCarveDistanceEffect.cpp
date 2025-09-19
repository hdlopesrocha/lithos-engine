#include "SDF.hpp"

WrappedVoronoiCarveDistanceEffect::WrappedVoronoiCarveDistanceEffect(WrappedSignedDistanceFunction * function, float amplitude, float cellSize, glm::vec3 offset, float brightness, float contrast) : WrappedSignedDistanceEffect(NULL), amplitude(amplitude), cellSize(cellSize), offset(offset), brightness(brightness), contrast(contrast) {
    this->setFunction(function);
}

WrappedVoronoiCarveDistanceEffect::~WrappedVoronoiCarveDistanceEffect() {

}

const char* WrappedVoronoiCarveDistanceEffect::getLabel() const {
    return "Voronoi Carve";
}

float WrappedVoronoiCarveDistanceEffect::distance(const glm::vec3 p, const Transformation &model) {
    glm::vec3 pp = p + offset; // apply offset
    float d = function->distance(p, model);

    float noise = SDF::voronoi3D(pp , cellSize, 0);
    return d - amplitude * Math::brightnessAndContrast(noise, brightness, contrast);
}

ContainmentType WrappedVoronoiCarveDistanceEffect::check(const BoundingCube &cube, const Transformation &model, float bias) const {
    return WrappedSignedDistanceEffect::check(cube, model, bias+amplitude);
};

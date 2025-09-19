#include "SDF.hpp"

WrappedVoronoiDistortDistanceEffect::WrappedVoronoiDistortDistanceEffect(WrappedSignedDistanceFunction * function, float amplitude, float cellSize, glm::vec3 offset) : WrappedSignedDistanceEffect(NULL), amplitude(amplitude), cellSize(cellSize), offset(offset) {
    this->setFunction(function);
}

WrappedVoronoiDistortDistanceEffect::~WrappedVoronoiDistortDistanceEffect() {

}

const char* WrappedVoronoiDistortDistanceEffect::getLabel() const {
    return "Voronoi Distort";
}

float WrappedVoronoiDistortDistanceEffect::distance(const glm::vec3 p, const Transformation &model) {
    glm::vec3 pp = p + offset; // apply offset
    float d = function->distance(p, model);
    return d + amplitude * SDF::voronoi3D(pp, cellSize, 0);
}

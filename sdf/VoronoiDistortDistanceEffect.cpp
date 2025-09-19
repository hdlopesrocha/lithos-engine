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
    float seed = 0.0f;
    glm::vec3 warp = 0.5f * glm::vec3(
        stb_perlin_noise3(pp.y, pp.z, seed, 0,0,0),
        stb_perlin_noise3(pp.z, pp.x, seed+10.0f, 0,0,0),
        stb_perlin_noise3(pp.x, pp.y, seed+20.0f, 0,0,0)
    );

    float v = SDF::voronoi3D(pp + warp, cellSize, 0);

    return d - amplitude * v;
}

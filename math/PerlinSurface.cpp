#include "math.hpp"



PerlinSurface::PerlinSurface(float amplitude, float frequency, float offset) {
    this->amplitude = amplitude;
    this->frequency = frequency;
    this->offset = offset;
}

float PerlinSurface::getHeightAt(float x, float z) const {

    float noise = stb_perlin_noise3(x* frequency, 0, z*frequency, 0, 0, 0);
    noise = offset + amplitude * noise;
    return noise;
}

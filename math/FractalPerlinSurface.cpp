#include "math.hpp"


FractalPerlinSurface::FractalPerlinSurface(float amplitude, float frequency, float offset) : PerlinSurface(amplitude, frequency, offset){
}

float FractalPerlinSurface::getHeightAt(float x, float z) const {
    float noise = 0;
    float weight = 1.0;
    float total = 0.0;
    float f = frequency;
    int octaves = 8;
    for(int o = 0 ; o < octaves ; ++o) {
        PerlinSurface perlin(amplitude, f, offset);
        noise += perlin.getHeightAt(x,z) * weight;
        total += weight;
        weight *= 0.5;
        f *= 2;
    }

    noise /= total;

    return offset + amplitude * noise;
}

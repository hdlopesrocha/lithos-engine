#include "tools.hpp"

WaveSurface::WaveSurface(float amplitude, float offset , float frequency) {
    this->amplitude = amplitude;
    this->offset = offset;
    this->frequency = frequency;
};

float WaveSurface::getHeightAt(float x, float z) const {
    return offset + amplitude * sin(frequency*x)*cos(frequency*z);
};

#include "math.hpp"


PerlinSurface::PerlinSurface(float amplitude, float frequency, float offset) {
    this->amplitude = amplitude;
    this->frequency = frequency;
    this->offset = offset;
}

float PerlinSurface::getHeightAt(float x, float y, float z) {
    float noise = db::perlin(double(x) * frequency, double(y) * frequency ,double(z) *frequency);
    return offset + amplitude * noise;
}




FractalPerlinSurface::FractalPerlinSurface(float amplitude, float frequency, float offset) : PerlinSurface(amplitude, frequency, offset){
}

float FractalPerlinSurface::getHeightAt(float x, float y, float z) {
    float noise = 0;
    float weight = 1.0;
    float total = 0.0;
    float f = frequency;
    int octaves = 8;
    for(int o = 0 ; o < octaves ; ++o) {
        PerlinSurface perlin(amplitude, f, offset);
        noise += perlin.getHeightAt(x,y,z) * weight;
        total += weight;
        weight *= 0.5;
        f *= 2;
    }

    noise /= total;

    return offset + amplitude * noise;
}




GradientPerlinSurface::GradientPerlinSurface(float amplitude, float frequency, float offset) : PerlinSurface(amplitude, frequency, offset){

}

float GradientPerlinSurface::getHeightAt(float x, float y,float z) {
    float noise = 0;
    float weight = 1.0;
    float total = 0.0;
    float f = frequency;

    for(int i = 0 ; i < 6 ; ++i) {
        PerlinSurface perlin(1 , f, 0);
        glm::vec3 n = perlin.getNormal(x,z, 0.5);
    
        float m = 1.0f -Math::clamp(glm::abs(glm::dot(glm::vec3(0,1,0), n)), 0.0f, 1.0f);
        float s = glm::pow(glm::e<float>(),- 1280.0f*m);


        noise += s*perlin.getHeightAt(x,y,z) * weight;
        total +=  weight;
        weight *= 0.5;

        f *= 2;
    }
    
    noise /= total;


    float beachLevel = 0.1;
    float divisions = 3;
    // Create beach
    if(noise < 0.1){
        noise = beachLevel+ (noise - beachLevel) / divisions;
    }


    return offset + amplitude * noise;
}


#include "math.hpp"


GradientPerlinSurface::GradientPerlinSurface(float amplitude, float frequency, float offset) : PerlinSurface(amplitude, frequency, offset){

}
long callsToGradientPerlinSurfaceGetHeight = 0;
float GradientPerlinSurface::getHeightAt(float x,float z) const   {
    float noise = 0;
    float weight = 1.0;
    float total = 0.0;
    float f = frequency;
    int octaves = 16;
    
    for(int i = 0 ; i < octaves ; ++i) {
        PerlinSurface perlin(1 , f, 0);
        glm::vec3 n = perlin.getNormal(x,z, 0.1);
    
        float m = 1.0f -Math::clamp(glm::abs(glm::dot(glm::vec3(0,1,0), n)), 0.0f, 1.0f);
        float s = glm::pow(glm::e<float>(),-2.0f*m);


        noise += s*perlin.getHeightAt(x,z) * weight;
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

    noise = offset + amplitude * noise;
    if(++callsToGradientPerlinSurfaceGetHeight%1000000 == 0 ){
        std::cout << "cg[" + std::to_string(callsToGradientPerlinSurfaceGetHeight) << "] = " << std::to_string(noise) << std::endl;
    }


    return noise;
}


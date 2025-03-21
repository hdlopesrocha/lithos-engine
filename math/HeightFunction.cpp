#include "math.hpp"

PerlinSurface::PerlinSurface(float amplitude, float frequency, float offset) {
    this->amplitude = amplitude;
    this->frequency = frequency;
    this->offset = offset;
}

float PerlinSurface::getHeightAt(float x, float z) const  {
    float noise = db::perlin(double(x) * frequency, double(0) * frequency ,double(z) *frequency);
    return offset + amplitude * noise;
}

FractalPerlinSurface::FractalPerlinSurface(float amplitude, float frequency, float offset) : PerlinSurface(amplitude, frequency, offset){
}

float FractalPerlinSurface::getHeightAt(float x, float z)  const {
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

GradientPerlinSurface::GradientPerlinSurface(float amplitude, float frequency, float offset) : PerlinSurface(amplitude, frequency, offset){

}

float GradientPerlinSurface::getHeightAt(float x,float z) const  {
    float noise = 0;
    float weight = 1.0;
    float total = 0.0;
    float f = frequency;

    for(int i = 0 ; i < 6 ; ++i) {
        PerlinSurface perlin(1 , f, 0);
        glm::vec3 n = perlin.getNormal(x,z, 0.5);
    
        float m = 1.0f -Math::clamp(glm::abs(glm::dot(glm::vec3(0,1,0), n)), 0.0f, 1.0f);
        float s = glm::pow(glm::e<float>(),- 1280.0f*m);


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


    return offset + amplitude * noise;
}

CachedHeightMapSurface::CachedHeightMapSurface( const HeightFunction &function, BoundingBox box, float delta) {
    this->box = box;
    this->delta = delta;
    glm::vec3 len = box.getLength();
    this->width = len.x / delta;
    this->height = len.z / delta;
    this->data = std::vector<std::vector<float>>(width, std::vector<float>(height)); 

    for(int i=0; i<width; ++i) {
        for(int j=0; j<height; ++j) {
            float x = i * delta + box.getMinX();
            float z = j * delta + box.getMinZ();
            this->data[i][j] = function.getHeightAt(x, z);
        }	
    }
}

float CachedHeightMapSurface::getData(int x, int z) const {
    return this->data[Math::clamp(x, 0, this->width-1)][Math::clamp(z, 0, this->height-1)];
}

float CachedHeightMapSurface::getHeightAt(float x, float z) const {
    // bilinear interpolation
    glm::vec3 len = box.getLength();


    float px = Math::clamp((x-box.getMinX())/len.x, 0.0, 1.0);
    float pz = Math::clamp((z-box.getMinZ())/len.z, 0.0, 1.0);
    int ix = floor(px * width);
    int iz = floor(pz * height);

    float qx = (px * width) - ix;
    float qz = (pz * height) - iz;

    float q11 = getData(ix, iz);
    float q21 = getData(ix+1, iz);
    float q12 = getData(ix, iz+1);
    float q22 = getData(ix+1, iz+1);

    float y1 = (1.0 - qx)*q11 + (qx)*q21;
    float y2 = (1.0 - qx)*q12 + (qx)*q22;

    float y = (1.0 - qz)*y1 + (qz)*y2;

    return y;
}
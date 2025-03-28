#include "math.hpp"

CachedHeightMapSurface::CachedHeightMapSurface(const HeightFunction &function, BoundingBox box, float delta) {
    this->box = box;
    glm::vec3 len = box.getLength();
    
    
    this->width = len.x/delta;
    this->height = len.z/delta;
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
long callsToGetHeightAtCached = 0;
float CachedHeightMapSurface::getHeightAt(float x, float z) const  {
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

    if(++callsToGetHeightAtCached%10000 == 0 ){
      //  std::cout << "ch[" + std::to_string(callsToGetHeightAtCached) << "] = " << std::to_string(y) << std::endl;
    }
    return Math::clamp( y, box.getMinY(), box.getMaxY());
}
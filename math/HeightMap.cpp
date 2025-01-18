#include "math.hpp"


HeightMap::HeightMap(glm::vec3 min, glm::vec3 max, int width, int height) {
    this->width = width;
    this->height = height;
    this->min = min;
	this->max = max;
    this->data = std::vector<std::vector<float>>(width, std::vector<float>(height)); 
    for(int x=0 ; x < width ; ++x){
        for(int z=0 ; z < height ; ++z){
            float h = Math::clamp((float ) ((sin((10.0*x)/width)*cos((10.0*z)/height)+1.0)/(2.0)), 0.0f, 1.0f);
            this->data[x][z] = h*0.6 + 0.2;
        }           
    }

}

HeightMap::HeightMap() {
    this->width = 0;
    this->height = 0;
    this->min = glm::vec3(0,0,0);
    this->max = glm::vec3(0,0,0);
}

glm::vec3 HeightMap::getMin() {
    return min;
}

glm::vec3 HeightMap::getMax() {
    return max;
}

glm::vec3 HeightMap::getLength() {
    return max - min;
}

glm::vec3 HeightMap::getCenter() {
    return (min+max)*0.5f;
}

float HeightMap::getData(int x, int z) {
    return this->data[Math::clamp(x, 0, this->width-1)][Math::clamp(z, 0, this->height-1)];
}

glm::ivec2 HeightMap::getIndexes(float x, float z) {
    glm::vec3 len = getLength();
    float px =(x-min.x)/len.x;
    float pz =(z-min.z)/len.z;
    int ix = Math::clamp((int)floor(px * this->width), 0, width-1);
    int iz = Math::clamp((int)floor(pz * this->height), 0, height-1);
    return glm::ivec2(ix, iz);
}

glm::vec2 HeightMap::getHeightRangeBetween(BoundingCube cube) {
    glm::ivec2 minIdx = getIndexes(cube.getMin().x, cube.getMin().z);
    glm::ivec2 maxIdx = getIndexes(cube.getMax().x, cube.getMax().z);
    glm::vec2 range = glm::vec2(getData(minIdx[0], minIdx[1]));

    for(int x = minIdx[0] ; x <= maxIdx[0]; ++x) {
        for(int z = minIdx[1] ; z <= maxIdx[1]; ++z) {
            float h = getData(x,z);
            range[0] = h < range[0] ? h : range[0];
            range[1] = h > range[1] ? h : range[1];
        }        
    }

    glm::vec3 len = getLength();
    range[0] = min[1] + range[0] * len[1];
    range[1] = min[1] + range[1] * len[1]; 

    return range;
}

float HeightMap::getHeightAt(float x, float z) {
    // bilinear interpolation
    glm::vec3 len = getLength();

    float px = Math::clamp((x-min.x)/len.x, 0.0, 1.0);
    float pz = Math::clamp((z-min.z)/len.z, 0.0, 1.0);
    int ix = floor(px * this->width);
    int iz = floor(pz * this->height);

    float qx = (px * this->width) - ix;
    float qz = (pz * this->height) - iz;

    float q11 = getData(ix, iz);
    float q21 = getData(ix+1, iz);
    float q12 = getData(ix, iz+1);
    float q22 = getData(ix+1, iz+1);

    float y1 = (1.0 - qx)*q11 + (qx)*q21;
    float y2 = (1.0 - qx)*q12 + (qx)*q22;

    float y = (1.0 - qz)*y1 + (qz)*y2;

    return min.y + y * len.y;
}

glm::vec3 getShift(int i) {
	return glm::vec3( ((i >> 0) % 2) , ((i >> 2) % 2) , ((i >> 1) % 2));
}

glm::vec3 HeightMap::getPoint(BoundingCube cube) {
    glm::vec3 v = cube.getCenter();
    float h = getHeightAt(v.x,v.z);
    if( Math::isBetween(h, cube.getMinY(), cube.getMaxY())){
        return glm::vec3(v.x, h, v.z);
    }  
   
    for(int i =0; i < 4 ; ++i) {
        v = cube.getMin() + cube.getLength() * getShift(i);
        h = getHeightAt(v.x,v.z);
        if( Math::isBetween(h, cube.getMinY(), cube.getMaxY())){
            return glm::vec3(v.x, h, v.z);
        }
    }
    return cube.getCenter();
}

float HeightMap::getMaxX() {
    return max.x;
}

float HeightMap::getMaxY() {
    return max.y;
}

float HeightMap::getMaxZ() {
    return max.z;
}

float HeightMap::getMinX() {
    return min.x;
}

float HeightMap::getMinY() {
    return min.y;
}

float HeightMap::getMinZ() {
    return min.z;
}

void HeightMap::setMin(glm::vec3 v) {
    this->min = v;
}

void HeightMap::setMax(glm::vec3 v) {
    this->max = v;
}


bool HeightMap::contains(glm::vec3 point){
    BoundingBox box(min, max);
    float h = getHeightAt(point.x, point.z);

    return box.contains(point) && Math::isBetween(point.y, min.y, h);
}


bool HeightMap::hitsBoundary(BoundingCube cube) {
    BoundingBox box(min, max);

    ContainmentType result = box.test(cube);
    bool allPointsUnderground = true;


    for(int i = 0; i<8 ; ++i) {
        glm::vec3 p = cube.getMin() + cube.getLength() * getShift(i);
        float y = getHeightAt(p.x, p.y);
        if(y < p.y) {
            allPointsUnderground = false;
            break;
        }
    }

    return result == ContainmentType::Intersects && allPointsUnderground;
}

ContainmentType HeightMap::test(BoundingCube cube) {
    BoundingBox box(min, max);

    ContainmentType result = box.test(cube);


    if(result != ContainmentType::Disjoint && result != ContainmentType::IsContained) {
        glm::vec2 range = getHeightRangeBetween(cube);
     //   std::cout << range[0] << " ! " << range[1] << std::endl;
        BoundingBox minBox(getMin(), glm::vec3(getMax().x, range[0], getMax().z ));
        BoundingBox maxBox(getMin(), glm::vec3(getMax().x, range[1], getMax().z ));
        
        ContainmentType minResult = minBox.test(cube);
        ContainmentType maxResult = maxBox.test(cube);
       
        if(minResult == ContainmentType::Contains){       
            result = ContainmentType::Contains;
        } else if(maxResult == ContainmentType::Disjoint){
            result = ContainmentType::Disjoint;
        } else {
            result = ContainmentType::Intersects;
        }
   }

    return result;
}


#include "math.hpp"


HeightMap::HeightMap(glm::vec3 min, glm::vec3 max, int width, int height, float step) : BoundingBox(min, max){
    this->width = width;
    this->height = height;
    this->step = step;
}



HeightMap::HeightMap() : BoundingBox(){
    this->width = 0;
    this->height = 0;
}

float HeightMap::getHeightAt(float x, float z) {
    glm::vec3 len = getLength();
    float amplitude = 10;
    float offset = -36;
    float frequency = 1.0/10.0;

    return offset + amplitude * sin(frequency*x)*cos(frequency*z);
}

glm::vec2 HeightMap::getHeightRangeBetween(BoundingCube cube) {
    glm::vec2 range = glm::vec2(getHeightAt(cube.getCenter().x,cube.getCenter().z));

    for(float x = cube.getMinX() ; x <= cube.getMaxX(); x+=step) {
        for(float z = cube.getMinZ() ; z <= cube.getMaxZ(); z+=step) {
            float h = getHeightAt(x,z);
            range[0] = h < range[0] ? h : range[0];
            range[1] = h > range[1] ? h : range[1];
        }        
    }

    return range;
}

glm::vec3 HeightMap::getNormalAt(float x, float z) {
    // bilinear interpolation


    float q11 = getHeightAt(x, z);
    float q21 = getHeightAt(x+step, z);
    float q12 = getHeightAt(x, z+step);

    glm::vec3 v11 = glm::vec3(0, q11, 0);
    glm::vec3 v21 = glm::vec3(step, q21, 0);
    glm::vec3 v12 = glm::vec3(0, q12, step);

    glm::vec3 n21 = glm::normalize(v21 -v11 );
    glm::vec3 n12 = glm::normalize(v12 -v11 );

    return glm::cross(n12,n21);
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

bool HeightMap::contains(glm::vec3 point){
    BoundingBox box(getMin(), getMax());
    float h = getHeightAt(point.x, point.z);

    return box.contains(point) && Math::isBetween(point.y, getMinY(), h);
}

bool HeightMap::isContained(BoundingCube p) {
    BoundingBox box(getMin(), getMax());
    return p.contains(box);
}
    
bool HeightMap::hitsBoundary(BoundingCube cube) {
    BoundingBox box(getMin(), getMax());

    ContainmentType result = box.test(cube);
    bool allPointsUnderground = true;

    glm::vec2 h = getHeightRangeBetween(cube);
    for(int i = 0; i<8 ; ++i) {
        glm::vec3 p = cube.getMin() + cube.getLength() * getShift(i);
        if(h[0] <= p.y) {
            allPointsUnderground = false;
            break;
        }
    }

    return result == ContainmentType::Intersects && allPointsUnderground;
}

ContainmentType HeightMap::test(BoundingCube cube) {
    BoundingBox box(getMin(), getMax());

    ContainmentType result = box.test(cube);

    if(result != ContainmentType::Disjoint) {
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


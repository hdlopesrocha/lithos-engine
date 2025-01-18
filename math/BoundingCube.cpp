#include "math.hpp"


BoundingCube::BoundingCube(glm::vec3 min, float length) {
	this->min = min;
	this->length = length;
}

float BoundingCube::getLength() {
    return length;
}

glm::vec3 BoundingCube::getMin() {
    return min;
}

glm::vec3 BoundingCube::getMax() {
    return min+glm::vec3(length);
}

glm::vec3 BoundingCube::getCenter() {
    return min+glm::vec3(length*0.5);
}

float BoundingCube::getMaxX() {
    return min[0] + length;
}

float BoundingCube::getMaxY() {
    return min[1] + length;
}

float BoundingCube::getMaxZ() {
    return min[2] + length;
}

float BoundingCube::getMinX() {
    return min[0];
}

float BoundingCube::getMinY() {
    return min[1];
}

float BoundingCube::getMinZ() {
    return min[2];
}

void BoundingCube::setLength(float l) {
    this->length = l;
}

void BoundingCube::setMin(glm::vec3 v) {
    this->min = v;
}


bool BoundingCube::contains(glm::vec3 point){
    return 
        Math::isBetween(point[0], min[0], getMax()[0]) &&
        Math::isBetween(point[1], min[1], getMax()[1]) &&
        Math::isBetween(point[2], min[2], getMax()[2]);
}

bool BoundingCube::contains(BoundingSphere sphere){
    glm::vec3 minS = (sphere.center-glm::vec3(sphere.radius));
    BoundingCube cube(minS, sphere.radius*2.0);
    return contains(cube);
}

bool BoundingCube::contains(BoundingCube cube){

    glm::vec3 minC = cube.getMin();
    glm::vec3 maxC = cube.getMax();
    
    if ( getMin()[0] <=  minC[0] && maxC[0] <= getMax()[0] && 
            getMin()[1] <=  minC[1] && maxC[1] <= getMax()[1] && 
            getMin()[2] <=  minC[2] && maxC[2] <= getMax()[2] ) {
        return true;
    }
    return false;
}
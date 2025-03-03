#include "math.hpp"

BoundingCube::BoundingCube() : AbstractBoundingBox(glm::vec3(0)) {
	this->length = 0;
}

BoundingCube::BoundingCube(glm::vec3 min, float length) : AbstractBoundingBox(min) {
	this->length = length;
}

glm::vec3 BoundingCube::getMax() {
    return getMin()+glm::vec3(length);
}

float BoundingCube::getMaxX() {
    return getMinX() + length;
}

float BoundingCube::getMaxY() {
    return getMinY() + length;
}

float BoundingCube::getMaxZ() {
    return getMinZ() + length;
}

glm::vec3 BoundingCube::getLength() {
    return glm::vec3(length);
}

float BoundingCube::getLengthX() {
    return length;
}

float BoundingCube::getLengthY() {
    return length;
}

float BoundingCube::getLengthZ() {
    return length;
}

void BoundingCube::setLength(float l) {
    this->length = l;
}


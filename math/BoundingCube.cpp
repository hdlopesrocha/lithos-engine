#include "math.hpp"

BoundingCube::BoundingCube() : AbstractBoundingBox(glm::vec3(0)) {
	this->length = 0;
}

BoundingCube::BoundingCube(glm::vec3 min, float length) : AbstractBoundingBox(min) {
	this->length = length;
}

glm::vec3 BoundingCube::getMax() const {
    return getMin()+glm::vec3(length);
}

float BoundingCube::getMaxX() const {
    return getMinX() + length;
}

float BoundingCube::getMaxY() const {
    return getMinY() + length;
}

float BoundingCube::getMaxZ() const {
    return getMinZ() + length;
}

glm::vec3 BoundingCube::getLength() const {
    return glm::vec3(length);
}

float BoundingCube::getLengthX() const {
    return length;
}

float BoundingCube::getLengthY() const {
    return length;
}

float BoundingCube::getLengthZ() const {
    return length;
}

void BoundingCube::setLength(float l) {
    this->length = l;
}


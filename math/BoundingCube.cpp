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

BoundingCube BoundingCube::getChild(int i) const {
	float newLength = 0.5*getLengthX();
    return BoundingCube(getMin() + newLength * AbstractBoundingBox::getShift(i), newLength);
}

bool BoundingCube::operator<(const BoundingCube& other) const {
    if (getMinX() != other.getMinX()) return getMinX() < other.getMinX();
    if (getMinY() != other.getMinY()) return getMinY() < other.getMinY();
    if (getMinZ() != other.getMinZ()) return getMinZ() < other.getMinZ();
    return length < other.length;
}

bool BoundingCube::operator==(const BoundingCube& other) const {
    return getMinX() == other.getMinX() 
        && getMinY() == other.getMinY()
        && getMinZ() == other.getMinZ() 
        && length == other.length;
}
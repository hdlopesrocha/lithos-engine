#include "math.hpp"
#include "SDF.hpp"

BoundingBox::BoundingBox(glm::vec3 min, glm::vec3 max) : AbstractBoundingBox(min) {
	this->max = max;
}

BoundingBox::BoundingBox() : AbstractBoundingBox(glm::vec3(0.0)){
    this->max = glm::vec3(0,0,0);
}

glm::vec3 BoundingBox::getMax() const {
    return max;
}

glm::vec3 BoundingBox::getLength() const {
    return getMax() - getMin();
}

float BoundingBox::getMaxX() const {
    return max[0];
}

float BoundingBox::getMaxY() const {
    return max[1];
}

float BoundingBox::getMaxZ() const {
    return max[2];
}

float BoundingBox::getLengthX() const {
    return max[0] - getMinX();
}

float BoundingBox::getLengthY() const {
    return max[1] - getMinY();
}

float BoundingBox::getLengthZ() const {
    return max[2] - getMinZ();
}

void BoundingBox::setMax(glm::vec3 v) {
    this->max = v;
}

void BoundingBox::setMaxX(float v){
    this->max.x = v;
}

void BoundingBox::setMaxY(float v){
    this->max.y = v;
}

void BoundingBox::setMaxZ(float v){
    this->max.z = v;
}


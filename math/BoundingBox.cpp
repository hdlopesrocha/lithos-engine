#include "math.hpp"


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

BoxContainmentHandler::BoxContainmentHandler(BoundingBox box, TextureBrush * b) : ContainmentHandler(){
    this->box = box;
    this->brush = b;
}

glm::vec3 BoxContainmentHandler::getCenter() const {
    return box.getCenter();
}

bool BoxContainmentHandler::contains(const glm::vec3 p) const {
    return box.contains(p);
}

bool BoxContainmentHandler::isContained(const BoundingCube &cube) const {
    return cube.contains(box);
}

ContainmentType BoxContainmentHandler::check(const BoundingCube &cube) const {
    return box.test(cube); 
}

Vertex BoxContainmentHandler::getVertex(const BoundingCube &cube, ContainmentType solid, glm::vec3 previousPoint) const {
    glm::vec3 min = this->box.getMin();
    glm::vec3 max = this->box.getMax();
    Vertex vertex(glm::clamp(previousPoint, min, max));
    vertex.normal = Math::surfaceNormal(vertex.position, box);
    brush->paint(&vertex);	
    return vertex;
}

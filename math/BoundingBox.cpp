#include "math.hpp"


BoundingBox::BoundingBox(glm::vec3 min, glm::vec3 max) : AbstractBoundingBox(min) {
	this->max = max;
}

BoundingBox::BoundingBox() : AbstractBoundingBox(glm::vec3(0.0)){
    this->max = glm::vec3(0,0,0);
}

glm::vec3 BoundingBox::getMax() {
    return max;
}

glm::vec3 BoundingBox::getLength() {
    return getMax() - getMin();
}

float BoundingBox::getMaxX() {
    return max[0];
}

float BoundingBox::getMaxY() {
    return max[1];
}

float BoundingBox::getMaxZ() {
    return max[2];
}

float BoundingBox::getLengthX() {
    return max[0] - getMinX();
}

float BoundingBox::getLengthY() {
    return max[1] - getMinY();
}

float BoundingBox::getLengthZ() {
    return max[2] - getMinZ();
}

void BoundingBox::setMax(glm::vec3 v) {
    this->max = v;
}

BoxContainmentHandler::BoxContainmentHandler(BoundingBox box, TextureBrush * b) : ContainmentHandler(){
    this->box = box;
    this->brush = b;
}

glm::vec3 BoxContainmentHandler::getCenter() {
    return box.getCenter();
}

bool BoxContainmentHandler::contains(glm::vec3 p) {
    return box.contains(p);
}

bool BoxContainmentHandler::isContained(BoundingCube &cube) {
    return cube.contains(box);
}

ContainmentType BoxContainmentHandler::check(BoundingCube &cube) {
    return box.test(cube); 
}

Vertex BoxContainmentHandler::getVertex(BoundingCube &cube, ContainmentType solid, glm::vec3 previousPoint) {
    glm::vec3 min = this->box.getMin();
    glm::vec3 max = this->box.getMax();
    Vertex vertex(glm::clamp(previousPoint, min, max));
    vertex.normal = Math::surfaceNormal(vertex.position, box);
    brush->paint(&vertex);	
    return vertex;
}

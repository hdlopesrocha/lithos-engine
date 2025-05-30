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

BoxContainmentHandler::BoxContainmentHandler(BoundingBox box, const TexturePainter &b) : ContainmentHandler(), box(box), brush(b){

}

float BoxContainmentHandler::distance(const glm::vec3 p) const {
    glm::vec3 len = box.getLength()*0.5f;
    glm::vec3 pos = p - box.getCenter();
    return SDF::box(pos, len);
}

bool BoxContainmentHandler::isContained(const BoundingCube &cube) const {
    return cube.contains(box);
}

ContainmentType BoxContainmentHandler::check(const BoundingCube &cube) const {
    return box.test(cube); 
}

Vertex BoxContainmentHandler::getVertex(const BoundingCube &cube, glm::vec3 previousPoint) const {
    glm::vec3 min = this->box.getMin();
    glm::vec3 max = this->box.getMax();
    Vertex vertex(glm::clamp(previousPoint, min, max));
    vertex.normal = Math::surfaceNormal(vertex.position, box);
    brush.paint(vertex);	
    return vertex;
}

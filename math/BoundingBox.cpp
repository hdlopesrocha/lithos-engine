#include "math.hpp"


BoundingBox::BoundingBox(glm::vec3 min, glm::vec3 max) {
	this->min = min;
	this->max = max;
}

BoundingBox::BoundingBox() {
    this->min = glm::vec3(0,0,0);
    this->max = glm::vec3(0,0,0);
}

glm::vec3 BoundingBox::getMin() {
    return min;
}

glm::vec3 BoundingBox::getMax() {
    return max;
}

glm::vec3 BoundingBox::getLength() {
    return max - min;
}

glm::vec3 BoundingBox::getCenter() {
    return (min+max)*0.5f;
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

float BoundingBox::getMinX() {
    return min[0];
}

float BoundingBox::getMinY() {
    return min[1];
}

float BoundingBox::getMinZ() {
    return min[2];
}

void BoundingBox::setMin(glm::vec3 v) {
    this->min = v;
}

void BoundingBox::setMax(glm::vec3 v) {
    this->max = v;
}


bool BoundingBox::contains(glm::vec3 point){
    return 
        Math::isBetween(point[0], min[0], max[0]) &&
        Math::isBetween(point[1], min[1], max[1]) &&
        Math::isBetween(point[2], min[2], max[2]);
}

ContainmentType BoundingBox::test(BoundingCube cube) {
    ContainmentType result;
    result = ContainmentType::Intersects;
    glm::vec3 min1 = cube.getMin();
    glm::vec3 max1 = cube.getMax();
    glm::vec3 min2 = getMin();
    glm::vec3 max2 = getMax();


    // Classify corners
    unsigned char outterMask = 0;

    for(int i=0; i < 8; ++i) {
        glm::vec3 sh = Octree::getShift(i);
        glm::vec3 p1(min1 + sh*cube.getLength());
        glm::vec3 p2(min2 + sh*getLength());

        if(contains(p1)){
            outterMask |= (1 << i); 
        }
    } 
   
    // Classifify type
 
    if(outterMask == 0xff) {
        result = ContainmentType::Contains;
    }
    else {
        for(int i=0 ; i < 3 ; ++i){
            if(    (min1[i] <= min2[i] && min2[i] <= max1[i]) 
                || (min1[i] <= max2[i] && max2[i] <= max1[i]) 
                || (min2[i] <= min1[i] && min1[i] <= max2[i]) 
                || (min2[i] <= max1[i] && max1[i] <= max2[i])){
                // overlaps in one dimension
            } else {
                result = ContainmentType::Disjoint;
                break;
            }
        }
    }
    return result;
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

bool BoxContainmentHandler::isContained(BoundingCube p) {
    return p.contains(box);
}

ContainmentType BoxContainmentHandler::check(BoundingCube cube) {
    return box.test(cube); 
}

Vertex BoxContainmentHandler::getVertex(BoundingCube cube, ContainmentType solid) {
    Vertex vertex(cube.getCenter());

    glm::vec3 min = this->box.getMin();
    glm::vec3 max = this->box.getMax();
    glm::vec3 c = cube.getCenter();

    vertex.position = glm::clamp(c, min, max);
    vertex.normal = Math::surfaceNormal(vertex.position, box);
    brush->paint(&vertex);	
    return vertex;
}

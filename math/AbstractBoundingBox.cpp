#include "math.hpp"


AbstractBoundingBox::AbstractBoundingBox(glm::vec3 min) : min(min) {
}

AbstractBoundingBox::AbstractBoundingBox() :min(glm::vec3(0,0,0)){

}

glm::vec3 AbstractBoundingBox::getMin() const {
    return min;
}

float AbstractBoundingBox::getMinX() const {
    return min[0];
}

float AbstractBoundingBox::getMinY() const {
    return min[1];
}

float AbstractBoundingBox::getMinZ() const {
    return min[2];
}

void AbstractBoundingBox::setMin(glm::vec3 v) {
    this->min = v;
}

glm::vec3 AbstractBoundingBox::getCenter() const {
    return (getMin()+getMax())*0.5f;
}

bool AbstractBoundingBox::contains(const glm::vec3 &point) const {
    return 
        Math::isBetween(point[0], min[0], getMaxX()) &&
        Math::isBetween(point[1], min[1], getMaxY()) &&
        Math::isBetween(point[2], min[2], getMaxZ());
}


bool AbstractBoundingBox::contains(const BoundingSphere &sphere) const {
    glm::vec3 minS = (sphere.center-glm::vec3(sphere.radius));
    BoundingCube cube(minS, sphere.radius*2.0);
    return contains(cube);
}


bool AbstractBoundingBox::contains(const AbstractBoundingBox &box) const {

    glm::vec3 minC = box.getMin();
    glm::vec3 maxC = box.getMax();
    
    if ( getMin()[0] <=  minC[0] && maxC[0] <= getMax()[0] && 
            getMin()[1] <=  minC[1] && maxC[1] <= getMax()[1] && 
            getMin()[2] <=  minC[2] && maxC[2] <= getMax()[2] ) {
        return true;
    }
    return false;
}

ContainmentType AbstractBoundingBox::test(const AbstractBoundingBox &cube) const {
    ContainmentType result;
    result = ContainmentType::Intersects;
    glm::vec3 min1 = cube.getMin();
    glm::vec3 max1 = cube.getMax();
    glm::vec3 min2 = getMin();
    glm::vec3 max2 = getMax();


    // Classify corners
    unsigned char outterMask = 0;

    for(int i=0; i < 8; ++i) {
        glm::vec3 sh = AbstractBoundingBox::getShift(i);
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

bool AbstractBoundingBox::intersects(const AbstractBoundingBox& box) const {
    glm::vec3 halfSize(getLength() / 2.0f);
    glm::vec3 minCube = getCenter() - halfSize;
    glm::vec3 maxCube = getCenter() + halfSize;

    glm::vec3 minBox = box.getMin();
    glm::vec3 maxBox = box.getMax();

    for (int i = 0; i < 3; ++i) {
        if (maxCube[i] < minBox[i] || minCube[i] > maxBox[i])
            return false;
    }

    return true;
}

glm::vec3 AbstractBoundingBox::getShift(int i) {
    return glm::vec3(4 & i? 1:0, 2 & i? 1:0, 1 & i? 1:0);
}


glm::vec3 AbstractBoundingBox::getShift3(int i) {
    return glm::vec3((i / 9) - 1, (i / 3) % 3 - 1, i % 3 - 1);
}


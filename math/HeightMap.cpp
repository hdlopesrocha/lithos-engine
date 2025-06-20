#include "math.hpp"
#include "SDF.hpp"

HeightMap::HeightMap(const HeightFunction &func, BoundingBox box, float step)  : BoundingBox(box), func(func), step(step){

}


glm::vec2 HeightMap::getHeightRangeBetween(const BoundingCube &cube) const {
    glm::vec2 range = glm::vec2(func.getHeightAt(cube.getCenter().x,cube.getCenter().z));

    for(float x = cube.getMinX() ; x <= cube.getMaxX(); x+=step) {
        for(float z = cube.getMinZ() ; z <= cube.getMaxZ(); z+=step) {
            float h = func.getHeightAt(x,z);
            range[0] = h < range[0] ? h : range[0];
            range[1] = h > range[1] ? h : range[1];
        }        
    }

    return range;
}

glm::vec3 HeightFunction::getNormal(float x, float z, float delta)  const {
    float q11 = getHeightAt(x,z);
    float q21 = getHeightAt(x+delta, z);
    float q12 = getHeightAt(x, z+delta);

    glm::vec3 v11 = glm::vec3(0, q11, 0);
    glm::vec3 v21 = glm::vec3(delta, q21, 0);
    glm::vec3 v12 = glm::vec3(0, q12, delta);

    glm::vec3 n21 = glm::normalize(v21 -v11 );
    glm::vec3 n12 = glm::normalize(v12 -v11 );

    return glm::cross(n12,n21);
}

glm::vec3 HeightMap::getNormalAt(float x, float z) const {
    return func.getNormal(x,z,step);
}

glm::vec3 getShift(int i) {
	return glm::vec3( ((i >> 0) % 2) , ((i >> 2) % 2) , ((i >> 1) % 2));
}

void HeightMap::getPoint(const BoundingCube &cube, glm::vec3 &p) const {
    glm::vec3 v = cube.getCenter();
    float h = func.getHeightAt(v.x,v.z);
    p= glm::vec3(v.x, h, v.z);
}

bool HeightMap::contains(const glm::vec3 &point) const {
    float h = func.getHeightAt(point.x, point.z);
    return BoundingBox::contains(point) && Math::isBetween(point.y, getMinY(), h);
}

float HeightMap::distance(const glm::vec3 p) const {
    float surfaceY = func.getHeightAt(p.x, p.z);
    glm::vec3 surface(p.x, surfaceY, p.z);
    float dy = p.y - surfaceY;
    float dist = glm::length(p - surface);
    return dy >= 0.0f ? dist : -dist;
}


bool HeightMap::isContained(const BoundingCube &p) const {
    return p.contains(*this);
}
    
bool HeightMap::hitsBoundary(const BoundingCube &cube) const {
    ContainmentType result = BoundingBox::test(cube);
    bool allPointsUnderground = true;

    glm::vec2 h = getHeightRangeBetween(cube);
    for(int i = 0; i<8 ; ++i) {
        glm::vec3 p = cube.getMin() + cube.getLengthX() * getShift(i);
        if(h[0] <= p.y) {
            allPointsUnderground = false;
            break;
        }
    }

    return result == ContainmentType::Intersects && allPointsUnderground;
}

ContainmentType HeightMap::test(const BoundingCube &cube) const {
    return BoundingBox::test(cube);
}

HeightMapContainmentHandler::HeightMapContainmentHandler(const HeightMap &m) : ContainmentHandler(), map(m) {

}

glm::vec3 HeightMapContainmentHandler::getCenter() const {
    return map.getCenter();
}

bool HeightMapContainmentHandler::isContained(const BoundingCube &p) const {
    return map.isContained(p);
}

float HeightMapContainmentHandler::intersection(const glm::vec3 a, const glm::vec3 b) const {
    return 0;	
} 

glm::vec3 HeightMapContainmentHandler::getNormal(const glm::vec3 pos) const {
    return map.getNormalAt(pos.x, pos.z);
}

ContainmentType HeightMapContainmentHandler::check(const BoundingCube &cube) const {
    return map.test(cube); 
}


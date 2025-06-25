#include "math.hpp"
#include "SDF.hpp"

HeightMap::HeightMap(const HeightFunction &func, BoundingBox box, float step)  : BoundingBox(box), func(func), step(step){

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

glm::vec3 getShift(int i) {
	return glm::vec3( ((i >> 0) % 2) , ((i >> 2) % 2) , ((i >> 1) % 2));
}

bool HeightMap::contains(const glm::vec3 &point) const {
    return BoundingBox::contains(point) ;
}

float HeightMap::distance(const glm::vec3 p) const {
    float surfaceY = func.getHeightAt(p.x, p.z);
    return p.y - surfaceY;
}


bool HeightMap::isContained(const BoundingCube &p) const {
    return p.contains(*this);
}
    
ContainmentType HeightMap::test(const BoundingCube &cube) const {
    return BoundingBox::test(cube);
}


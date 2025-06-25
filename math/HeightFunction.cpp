#include "math.hpp"

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
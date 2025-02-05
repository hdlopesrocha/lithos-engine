#include "math.hpp"

Plane::Plane(glm::vec3 normal, glm::vec3 point) {
    this->normal = normal;
    this->d = -(normal.x*point.x + normal.y*point.y + normal.z*point.z);

}

float Plane::distance(glm::vec3 point) {
    glm::vec3 m = normal*point;
    return abs(m.x+m.y+m.z+d)/glm::length(normal);
}
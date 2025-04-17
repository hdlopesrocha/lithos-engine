#include "math.hpp"

Plane::Plane(glm::vec3 normal, glm::vec3 point) {
    this->normal = normal;
    this->d = -(normal.x*point.x + normal.y*point.y + normal.z*point.z);
}

float Plane::distance(glm::vec3 &point) {
    return (glm::dot(normal, point) + d) / glm::length(normal);
}

ContainmentType Plane::test(AbstractBoundingBox &box) {
    glm::vec3 corners[8] = {
        glm::vec3(box.getMinX(), box.getMinY(), box.getMinZ()),
        glm::vec3(box.getMaxX(), box.getMinY(), box.getMinZ()),
        glm::vec3(box.getMinX(), box.getMaxY(), box.getMinZ()),
        glm::vec3(box.getMaxX(), box.getMaxY(), box.getMinZ()),
        glm::vec3(box.getMinX(), box.getMinY(), box.getMaxZ()),
        glm::vec3(box.getMaxX(), box.getMinY(), box.getMaxZ()),
        glm::vec3(box.getMinX(), box.getMaxY(), box.getMaxZ()),
        glm::vec3(box.getMaxX(), box.getMaxY(), box.getMaxZ())
    };

    bool hasPositive = false;
    bool hasNegative = false;

    for (int i = 0; i < 8; ++i) {
        float dist = distance(corners[i]);
        if (dist > 0) hasPositive = true;
        else if (dist < 0) hasNegative = true;

        if (hasPositive && hasNegative)
            return ContainmentType::Intersects; 
    }

    return hasNegative ? ContainmentType::Contains : ContainmentType::Disjoint;
}
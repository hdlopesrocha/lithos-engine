#include "math.hpp"
namespace {
    Vertex getVertex(glm::vec3 v, int plane) {
        return Vertex(v, glm::normalize(v), Math::triplanarMapping(v, plane), 0);
    }
}

BoxGeometry::BoxGeometry(const BoundingBox &box) : Geometry(true) {
    glm::vec3 corners[8] = {
        {box.getMinX(), box.getMinY(), box.getMinZ()}, // 0
        {box.getMaxX(), box.getMinY(), box.getMinZ()}, // 1
        {box.getMaxX(), box.getMaxY(), box.getMinZ()}, // 2
        {box.getMinX(), box.getMaxY(), box.getMinZ()}, // 3
        {box.getMinX(), box.getMinY(), box.getMaxZ()}, // 4
        {box.getMaxX(), box.getMinY(), box.getMaxZ()}, // 5
        {box.getMaxX(), box.getMaxY(), box.getMaxZ()}, // 6
        {box.getMinX(), box.getMaxY(), box.getMaxZ()}  // 7
    };

    addTriangle(corners[4], corners[5], corners[6]);
    addTriangle(corners[4], corners[6], corners[7]);

    addTriangle(corners[1], corners[0], corners[3]);
    addTriangle(corners[1], corners[3], corners[2]);

    addTriangle(corners[0], corners[4], corners[7]);
    addTriangle(corners[0], corners[7], corners[3]);

    addTriangle(corners[5], corners[1], corners[2]);
    addTriangle(corners[5], corners[2], corners[6]);

    addTriangle(corners[0], corners[1], corners[5]);
    addTriangle(corners[0], corners[5], corners[4]);

    addTriangle(corners[3], corners[7], corners[6]);
    addTriangle(corners[3], corners[6], corners[2]);
}

void BoxGeometry::addTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
    glm::vec3 v0 = b - a;
    glm::vec3 v1 = c - a;
    glm::vec3 n = glm::cross(v0, v1); 
    int plane = Math::triplanarPlane(a, n);

    addVertex(getVertex(a, plane));
    addVertex(getVertex(b, plane));
    addVertex(getVertex(c, plane));
}
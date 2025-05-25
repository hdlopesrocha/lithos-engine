#include "tools.hpp"

OctreeContainmentHandler::OctreeContainmentHandler(Octree * octree, BoundingBox box, const TexturePainter &b): brush(b) {
    this->octree = octree;
    this->box = box;
}

bool OctreeContainmentHandler::contains(const glm::vec3 p) const {
    ContainmentType tc = octree->contains(p);
    return box.contains(p) && tc != ContainmentType::Contains;
}

float OctreeContainmentHandler::distance(const glm::vec3 p) const {
    return 1;
}

bool OctreeContainmentHandler::isContained(const BoundingCube &p) const {
    return p.contains(box);
}

glm::vec3 OctreeContainmentHandler::getNormal(const glm::vec3 pos) const {
    glm::vec3 center = box.getCenter();

    glm::vec3 topFL(box.getMinX(), box.getMaxY(), box.getMinZ());
    glm::vec3 topFR(box.getMaxX(), box.getMaxY(), box.getMinZ()); 
    glm::vec3 topBL(box.getMinX(), box.getMaxY(), box.getMaxZ()); 
    glm::vec3 topBR(box.getMaxX(), box.getMaxY(), box.getMaxZ()); 

    auto pointIsBelowPlane = [](const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
        glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
        return glm::dot(p - a, normal) <= 0.0f;
    };

    bool inside =
        pointIsBelowPlane(pos, center, topFL, topFR) &&
        pointIsBelowPlane(pos, center, topFR, topBR) &&
        pointIsBelowPlane(pos, center, topBR, topBL) &&
        pointIsBelowPlane(pos, center, topBL, topFL);

    return inside ? glm::vec3(0, 1, 0) : glm::vec3(0, -1, 0);
}

ContainmentType OctreeContainmentHandler::check(const BoundingCube &cube) const {
    ContainmentType boxResult = box.test(cube);
    // TODO: fix extra volume created
    return boxResult;

    if (boxResult == ContainmentType::Disjoint)
        return ContainmentType::Disjoint;
        
    ContainmentType sourceResult = octree->contains(cube);



    if (sourceResult == ContainmentType::Disjoint)
        return boxResult;

    if (sourceResult == ContainmentType::Intersects)
        return ContainmentType::Intersects;

    return ContainmentType::Disjoint;
}

Vertex OctreeContainmentHandler::getVertex(const BoundingCube &cube, ContainmentType solid, glm::vec3 previousPoint) const {
    Vertex vtx;
    vtx.position = cube.getCenter();
    vtx.normal = getNormal(vtx.position);
    brush.paint(vtx);
    return vtx;
}

#include "tools.hpp"

OctreeContainmentHandler::OctreeContainmentHandler(Octree * octree, BoundingBox box, const TexturePainter &b): brush(b) {
    this->octree = octree;
    this->box = box;
}

glm::vec3 OctreeContainmentHandler::getCenter() const {
    return box.getCenter();
}

bool OctreeContainmentHandler::contains(const glm::vec3 p) const {
    ContainmentType tc = octree->contains(p);
    return box.contains(p) && tc != ContainmentType::Contains;
}

bool OctreeContainmentHandler::isContained(const BoundingCube &p) const {
    return p.contains(box);
}

glm::vec3 OctreeContainmentHandler::getNormal(const glm::vec3 pos) const {
    return pos.y < box.getMaxY() ? glm::vec3(0,-1,0) : glm::vec3(0,1,0);
}

ContainmentType OctreeContainmentHandler::check(const BoundingCube &cube) const {
    ContainmentType boxResult = box.test(cube);
//    if (boxResult == ContainmentType::Disjoint)
//        return ContainmentType::Disjoint;

    ContainmentType sourceResult = octree->contains(cube);
return sourceResult;



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

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
    ContainmentType ct0 = box.test(cube);

    if(ct0 != ContainmentType::Disjoint) {
        ContainmentType ct1 = octree->contains(cube);
        if(ct1 == ContainmentType::Disjoint) {
            return ct0;
        } else if(ct1 == ContainmentType::Intersects) {
            return ct1;
        }
    }
    return ContainmentType::Disjoint;
}

Vertex OctreeContainmentHandler::getVertex(const BoundingCube &cube, ContainmentType solid, glm::vec3 previousPoint) const {
    Vertex vtx;
    vtx.position = cube.getCenter();
    vtx.normal = getNormal(vtx.position);
    brush.paint(vtx);
    return vtx;
}

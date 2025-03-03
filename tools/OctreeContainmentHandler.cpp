#include "tools.hpp"

OctreeContainmentHandler::OctreeContainmentHandler(Octree * octree, BoundingBox box, TextureBrush * b) {
    this->brush = b;
    this->octree = octree;
    this->box = box;
}

glm::vec3 OctreeContainmentHandler::getCenter() {
    return box.getCenter();
}

bool OctreeContainmentHandler::contains(glm::vec3 p) {
    ContainmentType tc = octree->contains(p);
    return tc != ContainmentType::Contains && box.contains(p);
}

bool OctreeContainmentHandler::isContained(BoundingCube &p) {
    return p.contains(box);
}

glm::vec3 OctreeContainmentHandler::getNormal(glm::vec3 pos) {
    return Math::surfaceNormal(pos, box);
}

ContainmentType OctreeContainmentHandler::check(BoundingCube &cube) {
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

Vertex OctreeContainmentHandler::getVertex(BoundingCube &cube, ContainmentType solid, glm::vec3 previousPoint) {
    Vertex vtx;
    vtx.position = cube.getCenter();
    vtx.normal = getNormal(vtx.position);
    brush->paint(&vtx);
    return vtx;
}

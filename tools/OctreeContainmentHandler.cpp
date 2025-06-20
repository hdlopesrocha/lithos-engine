#include "tools.hpp"

OctreeContainmentHandler::OctreeContainmentHandler(Octree * octree, BoundingBox box) {
    this->octree = octree;
    this->box = box;
}

bool OctreeContainmentHandler::isContained(const BoundingCube &p) const {
    return p.contains(box);
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

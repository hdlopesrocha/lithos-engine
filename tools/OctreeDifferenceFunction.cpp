#include "tools.hpp"

OctreeDifferenceFunction::OctreeDifferenceFunction(Octree * tree, BoundingBox box):tree(tree), box(box) {

}

float OctreeDifferenceFunction::distance(const glm::vec3 p) const {
    glm::vec3 len = box.getLength()*0.5f;
    glm::vec3 pos = p - box.getCenter();
    return SDF::opSubtraction(
        SDF::box(pos, len),
        tree->getSdfAt(p)
    );
}
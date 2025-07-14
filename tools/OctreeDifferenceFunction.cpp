#include "tools.hpp"

OctreeDifferenceFunction::OctreeDifferenceFunction(Octree * tree, BoundingBox box):tree(tree), box(box) {

}

float OctreeDifferenceFunction::distance(const glm::vec3 p, Transformation model) const {
    glm::vec3 len = box.getLength()*0.5f;
    glm::vec3 pos = p - box.getCenter()+model.translate;
    return SDF::opSubtraction(
        SDF::box(pos, len),
        tree->getSdfAt(p)
    );
}

SdfType OctreeDifferenceFunction::getType() const {
    return SdfType::OCTREE_DIFFERENCE;
}

glm::vec3 OctreeDifferenceFunction::getCenter(Transformation model) const {
    return this->box.getCenter() + model.translate;
}
#include "tools.hpp"

OctreeDifferenceFunction::OctreeDifferenceFunction(Octree * tree, BoundingBox box, float bias):tree(tree), box(box), bias(bias) {

}

float OctreeDifferenceFunction::distance(const glm::vec3 p, const Transformation &model) {
    glm::vec3 len = box.getLength()*0.5f;
    glm::vec3 pos = p - box.getCenter()+model.translate;
    return SDF::opSubtraction(
        SDF::box(pos, len),
        tree->getSdfAt(p)+bias
    );
}

SdfType OctreeDifferenceFunction::getType() const {
    return SdfType::OCTREE_DIFFERENCE;
}

glm::vec3 OctreeDifferenceFunction::getCenter(const Transformation &model) const {
    return this->box.getCenter() + model.translate;
}
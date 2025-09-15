#include "SDF.hpp"


HeightMapDistanceFunction::HeightMapDistanceFunction(HeightMap * map) {
    this->map = map;
}

float HeightMapDistanceFunction::distance(const glm::vec3 p, const Transformation &model)  {
    glm::vec3 len = map->getLength()*0.5f;
    glm::vec3 pos = p - map->getCenter();

    float sdf = map->distance(p);


    float d = SDF::opIntersection(
        SDF::box(pos+model.translate, len),
        sdf
    );

    return d;
}

SdfType HeightMapDistanceFunction::getType() const {
    return SdfType::HEIGHTMAP; 
}

glm::vec3 HeightMapDistanceFunction::getCenter(const Transformation &model) const {
    return this->map->getCenter();
}


WrappedHeightMap::WrappedHeightMap(HeightMapDistanceFunction * function) : WrappedSignedDistanceFunction(function) {

}

WrappedHeightMap::~WrappedHeightMap() {

}

BoundingBox WrappedHeightMap::getBox(float bias) const {
    HeightMapDistanceFunction * f = (HeightMapDistanceFunction*) function;
    return BoundingBox(f->map->getMin()-glm::vec3(bias), f->map->getMax()+glm::vec3(bias));
}
    
ContainmentType WrappedHeightMap::check(const BoundingCube &cube, const Transformation &model, float bias) const {
    BoundingBox box = getBox(bias);
    return box.test(cube);
};

bool WrappedHeightMap::isContained(const BoundingCube &cube, const Transformation &model, float bias) const {
    BoundingBox box = getBox(bias);
    return cube.contains(box);
};

float WrappedHeightMap::getLength(const Transformation &model, float bias) const {
    HeightMapDistanceFunction * f = (HeightMapDistanceFunction*) function;
    return glm::distance(f->map->getMin(), f->map->getMax()) + bias;
};

void WrappedHeightMap::accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const {
    getBox(bias).accept(visitor);
}

const char* WrappedHeightMap::getLabel() const {
    return "Height Map";
}
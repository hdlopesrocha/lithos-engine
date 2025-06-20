#include "SDF.hpp"

glm::vec3 SDF::getPosition(float *sdf, const BoundingCube &cube) {
    std::vector<glm::vec3> positions;
    for(int i =0 ; i < 12 ; ++i) {
        glm::ivec2 edge = SDF_EDGES[i];
        float d0 = sdf[edge[0]];
        float d1 = sdf[edge[1]];
        if(d0 != d1) {
            glm::vec3 p0 = cube.getCorner(edge[0]);
            glm::vec3 p1 = cube.getCorner(edge[1]);
            float t = d0 / (d0 - d1);
            positions.push_back(p0 + t * (p1 - p0));
        }
    }
    
    glm::vec3 sum(0.0f);
    for(glm::vec3 position : positions) {
        sum += position;
    }
    return positions.size() > 0 ? sum / static_cast<float>(positions.size()) : cube.getCenter();
}

float SDF::opUnion( float d1, float d2 ) {
    return glm::min(d1,d2);
}

float SDF::opSubtraction( float d1, float d2 ) {
    return glm::max(-d1,d2);
}

float SDF::opIntersection( float d1, float d2 ) {
    return glm::max(d1,d2);
}

float SDF::opXor(float d1, float d2 ) {
    return glm::max(glm::min(d1,d2),-glm::max(d1,d2));
}

float SDF::box(glm::vec3 p, const glm::vec3 len) {
    glm::vec3 q = abs(p) - len;
    return glm::length(glm::max(q, glm::vec3(0.0))) + glm::min(glm::max(q.x,glm::max(q.y,q.z)),0.0f);
}

float SDF::sphere(glm::vec3 p, const float r) {
    return glm::length(p) - r;
}

float SDF::opSmoothUnion( float d1, float d2, float k ) {
    float h = glm::clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return glm::mix( d2, d1, h ) - k*h*(1.0-h);
}

float SDF::opSmoothSubtraction( float d1, float d2, float k ) {
    float h = glm::clamp( 0.5 - 0.5*(d2+d1)/k, 0.0, 1.0 );
    return glm::mix( d2, -d1, h ) + k*h*(1.0-h);
}

float SDF::opSmoothIntersection( float d1, float d2, float k ) {
    float h = glm::clamp( 0.5 - 0.5*(d2-d1)/k, 0.0, 1.0 );
    return glm::mix( d2, d1, h ) + k*h*(1.0-h);
}

BoxDistanceFunction::BoxDistanceFunction(const BoundingBox box): box(box) {
}

float BoxDistanceFunction::distance(const glm::vec3 p) const {
    glm::vec3 len = box.getLength()*0.5f;
    glm::vec3 pos = p - box.getCenter();
    return SDF::box(pos, len);
}

SphereDistanceFunction::SphereDistanceFunction(const BoundingSphere sphere): sphere(sphere) {
}

float SphereDistanceFunction::distance(const glm::vec3 p) const {
    glm::vec3 pos = p - sphere.center;
    return SDF::sphere(pos, sphere.radius);
}

HeightMapDistanceFunction::HeightMapDistanceFunction(const HeightMap &map):map(map) {
}

float HeightMapDistanceFunction::distance(const glm::vec3 p) const {
    return map.distance(p);
}
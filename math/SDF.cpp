#include "SDF.hpp"

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
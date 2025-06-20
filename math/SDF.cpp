#include "SDF.hpp"

glm::vec3 SDF::getPosition(float *sdf, const BoundingCube &cube) {
    // Early exit if there's no surface inside this cube
    bool allPositive = true;
    bool allNegative = true;
    for (int i = 0; i < 8; ++i) {
        bool positive = sdf[i] >= 0.0f;
        allPositive &= positive;
        allNegative &= !positive;
    }
    if (allPositive || allNegative) {
        return cube.getCenter();  // or return {} if invalid
    }

    std::vector<glm::vec3> positions;
    for (int i = 0; i < 12; ++i) {
        glm::ivec2 edge = SDF_EDGES[i];
        float d0 = sdf[edge[0]];
        float d1 = sdf[edge[1]];

		bool sign0 = d0 < 0.0f;
		bool sign1 = d1 < 0.0f;

        if (sign0 != sign1) {
            glm::vec3 p0 = cube.getCorner(edge[0]);
            glm::vec3 p1 = cube.getCorner(edge[1]);
            float t = d0 / (d0 - d1);  // Safe due to sign change
            positions.push_back(p0 + t * (p1 - p0));
        }
    }

    if (positions.empty())
        return cube.getCenter();  // fallback or invalid

    glm::vec3 sum(0.0f);
    for (const glm::vec3 &p : positions) {
        sum += p;
    }

    return sum / static_cast<float>(positions.size());
}

glm::vec3 SDF::getNormal(float* sdf, const BoundingCube& cube) {
    const float dx = cube.getLengthX(); // or half size if your sdf spacing is half
    const float inv2dx = 1.0f / (2.0f * dx);

    // Gradient approximation via central differences:
    float gx = (sdf[1] + sdf[5] + sdf[3] + sdf[7] - sdf[0] - sdf[4] - sdf[2] - sdf[6]) * 0.25f;
    float gy = (sdf[2] + sdf[3] + sdf[6] + sdf[7] - sdf[0] - sdf[1] - sdf[4] - sdf[5]) * 0.25f;
    float gz = (sdf[4] + sdf[5] + sdf[6] + sdf[7] - sdf[0] - sdf[1] - sdf[2] - sdf[3]) * 0.25f;

    glm::vec3 normal(gx, gy, gz);
    return glm::normalize(normal * inv2dx);
}

glm::vec3 SDF::getNormalFromPosition(float* sdf, const BoundingCube& cube, const glm::vec3& position) {
    glm::vec3 local = (position - cube.getMin()) / cube.getLength(); // Convert to [0,1]^3 within cube

    // Trilinear interpolation gradient
    float dx = (
        (1 - local.y) * (1 - local.z) * (sdf[1] - sdf[0]) +
        local.y * (1 - local.z) * (sdf[3] - sdf[2]) +
        (1 - local.y) * local.z * (sdf[5] - sdf[4]) +
        local.y * local.z * (sdf[7] - sdf[6])
    );

    float dy = (
        (1 - local.x) * (1 - local.z) * (sdf[2] - sdf[0]) +
        local.x * (1 - local.z) * (sdf[3] - sdf[1]) +
        (1 - local.x) * local.z * (sdf[6] - sdf[4]) +
        local.x * local.z * (sdf[7] - sdf[5])
    );

    float dz = (
        (1 - local.x) * (1 - local.y) * (sdf[4] - sdf[0]) +
        local.x * (1 - local.y) * (sdf[5] - sdf[1]) +
        (1 - local.x) * local.y * (sdf[6] - sdf[2]) +
        local.x * local.y * (sdf[7] - sdf[3])
    );

    return glm::normalize(glm::vec3(dx, dy, dz) / cube.getLength());
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
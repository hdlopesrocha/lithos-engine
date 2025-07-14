#include "SDF.hpp"

glm::vec3 SDF::getPosition(float *sdf, const BoundingCube &cube) {
    // Early exit if there's no surface inside this cube
    SpaceType eval = SDF::eval(sdf);
    if(eval != SpaceType::Surface) {
        return cube.getCenter();  // or some fallback value
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

    if (positions.empty()){
        std::cout << "Invalid point!" << std::endl;
        return cube.getCenter();  // fallback or invalid
    }
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
    return glm::max(d1,-d2);
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

float SDF::capsule(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r ) {
    glm::vec3 pa = p - a, ba = b - a;
    float h = glm::clamp( glm::dot(pa,ba)/glm::dot(ba,ba), 0.0f, 1.0f );
    return glm::length( pa - ba*h ) - r;
}

float SDF::sphere(glm::vec3 p, const float r) {
    return glm::length(p) - r;
}

float SDF::octahedron(glm::vec3 p, float s ) {
  p = abs(p);
  float m = p.x+p.y+p.z-s;
  glm::vec3 q;
       if( 3.0*p.x < m ) q = glm::vec3(p.x, p.y, p.z);
  else if( 3.0*p.y < m ) q = glm::vec3(p.y, p.z, p.x);
  else if( 3.0*p.z < m ) q = glm::vec3(p.z, p.x, p.y);
  else return m*0.57735027;
    
  float k = glm::clamp(0.5f*(q.z-q.y+s),0.0f,s); 
  return glm::length(glm::vec3(q.x,q.y-s+k,q.z-k)); 
}

float SDF::pyramid(glm::vec3 p, float h ){
    float m2 = h*h + 0.25;
    
    // symmetry
    p.x = glm::abs(p.x); // do p=abs(p) instead for double pyramid
    p.z = glm::abs(p.z); // do p=abs(p) instead for double pyramid
    if(p.z>p.x) {
        float t = p.x;
        p.x = p.z;
        p.z = t;
    }
    p.x -= 0.5f;
    p.z -= 0.5f;
	
    // project into face plane (2D)
    glm::vec3 q = glm::vec3( p.z, h*p.y-0.5f*p.x, h*p.x+0.5f*p.y);
        
    float s = glm::max(-q.x,0.0f);
    float t = glm::clamp( (q.y-0.5f*q.x)/(m2+0.25f), 0.0f, 1.0f );
    
    float a = m2*(q.x+s)*(q.x+s) + q.y*q.y;
	float b = m2*(q.x+0.5f*t)*(q.x+0.5f*t) + (q.y-m2*t)*(q.y-m2*t);
    
    float d2 = glm::max(-q.y,q.x*m2+q.y*0.5f) < 0.0 ? 0.0 : glm::min(a,b);
    
    // recover 3D and scale, and add sign
    return sqrt( (d2+q.z*q.z)/m2 ) * glm::sign(glm::max(q.z,-p.y));;
}


float SDF::opSmoothUnion( float d1, float d2, float k ) {
    float h = glm::clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return glm::mix( d2, d1, h ) - k*h*(1.0-h);
}

float SDF::opSmoothSubtraction( float d1, float d2, float k ) {
    float h = glm::clamp( 0.5 - 0.5*(d2+d1)/k, 0.0, 1.0 );
    return glm::mix( d1, -d2, h ) + k*h*(1.0-h);
}

float SDF::opSmoothIntersection( float d1, float d2, float k ) {
    float h = glm::clamp( 0.5 - 0.5*(d1-d2)/k, 0.0, 1.0 );
    return glm::mix( d1, d2, h ) + k*h*(1.0-h);
}

float SDF::interpolate(float * sdf, glm::vec3 position, BoundingCube cube) {
    glm::vec3 local = (position - cube.getMin()) / cube.getLength(); // Convert to [0,1]^3 within cube

    // Trilinear interpolation
    float c00 = glm::mix(sdf[0], sdf[1], local.x);
    float c01 = glm::mix(sdf[4], sdf[5], local.x);
    float c10 = glm::mix(sdf[2], sdf[3], local.x);
    float c11 = glm::mix(sdf[6], sdf[7], local.x);

    float c0 = glm::mix(c00, c10, local.y);
    float c1 = glm::mix(c01, c11, local.y);

    return glm::mix(c0, c1, local.z);
}

void SDF::getChildSDF(float * sdf, int i , float * result) {
    for (int corner = 0; corner < 8; ++corner) {
        float fx = ((corner & 4) ? 0.5f : 0.0f) + ((i & 4) ? 0.5f : 0.0f);
        float fy = ((corner & 2) ? 0.5f : 0.0f) + ((i & 2) ? 0.5f : 0.0f);
        float fz = ((corner & 1) ? 0.5f : 0.0f) + ((i & 1) ? 0.5f : 0.0f);

        float c00 = glm::mix(sdf[0], sdf[1], fx);
        float c01 = glm::mix(sdf[4], sdf[5], fx);
        float c10 = glm::mix(sdf[2], sdf[3], fx);
        float c11 = glm::mix(sdf[6], sdf[7], fx);

        float c0 = glm::mix(c00, c10, fy);
        float c1 = glm::mix(c01, c11, fy);

        result[corner] = glm::mix(c0, c1, fz);
    }
}

void SDF::copySDF(float * src, float * dst) {
    for (int corner = 0; corner < 8; ++corner) {
        dst[corner] = src[corner];
    }
}

BoxDistanceFunction::BoxDistanceFunction(glm::vec3 center, glm::vec3 length): center(center), length(length) {

}

float BoxDistanceFunction::distance(const glm::vec3 p, Transformation model) const {
    glm::vec3 pos = p - getCenter(model); // Move point into model space
    pos = glm::inverse(model.getRotation()) * pos;
    return SDF::box(pos/model.scale, length);
}

SdfType BoxDistanceFunction::getType() const {
    return SdfType::BOX;
}

glm::vec3 BoxDistanceFunction::getCenter(Transformation model) const {
    return this->center+model.translate;
}


SphereDistanceFunction::SphereDistanceFunction(glm::vec3 pos, float radius): center(pos), radius(radius) {
    
}

float SphereDistanceFunction::distance(const glm::vec3 p, Transformation model) const {
    glm::vec3 pos = p - getCenter(model); // Move point into model space
    pos = glm::inverse(model.getRotation()) * pos;
    return SDF::sphere(pos/model.scale, radius);
}

SdfType SphereDistanceFunction::getType() const {
    return SdfType::SPHERE;
}

glm::vec3 SphereDistanceFunction::getCenter(Transformation model) const {
    return this->center+model.translate;
}


CapsuleDistanceFunction::CapsuleDistanceFunction(glm::vec3 a, glm::vec3 b, float r) {
    this->a = a;
    this->b = b;
    this->radius = r;
}

float CapsuleDistanceFunction::distance(const glm::vec3 p, Transformation model) const {
    glm::vec3 pos = p - getCenter(model); // Move point into model space
    pos = glm::inverse(model.getRotation()) * pos;
    return SDF::capsule(pos/model.scale, a, b, radius);
}

SdfType CapsuleDistanceFunction::getType() const {
    return SdfType::CAPSULE;
}

glm::vec3 CapsuleDistanceFunction::getCenter(Transformation model) const {
    return 0.5f*(this->a+this->b)+model.translate;
}

HeightMapDistanceFunction::HeightMapDistanceFunction(const HeightMap &map):map(map) {
}

float HeightMapDistanceFunction::distance(const glm::vec3 p, Transformation model) const {
    glm::vec3 len = map.getLength()*0.5f;
    glm::vec3 pos = p - map.getCenter();

    float heightMapSDF = map.distance(p);
    return SDF::opIntersection(
        SDF::box(pos+model.translate, len),
        heightMapSDF
    );
}

SdfType HeightMapDistanceFunction::getType() const {
    return SdfType::HEIGHTMAP; 
}

glm::vec3 HeightMapDistanceFunction::getCenter(Transformation model) const {
    return this->map.getCenter();
}

OctahedronDistanceFunction::OctahedronDistanceFunction(glm::vec3 center, float radius): center(center), radius(radius) {
    
}

float OctahedronDistanceFunction::distance(const glm::vec3 p, Transformation model) const {
    glm::vec3 pos = p - getCenter(model); // Move point into model space
    pos = glm::inverse(model.getRotation()) * pos;
    return SDF::octahedron(pos/model.scale, radius);
}

SdfType OctahedronDistanceFunction::getType() const {
    return SdfType::OCTAHEDRON;
}

glm::vec3 OctahedronDistanceFunction::getCenter(Transformation model) const {
    return this->center+model.translate;
}


PyramidDistanceFunction::PyramidDistanceFunction(glm::vec3 base, float height): base(base), height(height) {
    
}

float PyramidDistanceFunction::distance(const glm::vec3 p, Transformation model) const {
    glm::vec3 pos = p - getCenter(model); // Move point into model space
    pos = glm::inverse(model.getRotation()) * pos;
    return SDF::pyramid(pos/model.scale, height);
}

SdfType PyramidDistanceFunction::getType() const {
    return SdfType::PYRAMID;
}

glm::vec3 PyramidDistanceFunction::getCenter(Transformation model) const {
    return base+model.translate;
}


SpaceType SDF::eval(float * sdf) {
    bool hasPositive = false;
    bool hasNegative = false;
    for (int i = 0; i < 8; ++i) {  
        if (sdf[i] >= 0.0f) {
            hasPositive = true;
        } else {
            hasNegative = true;
        }
    }
    return hasNegative && hasPositive ? SpaceType::Surface : (hasPositive ? SpaceType::Empty : SpaceType::Solid);
}

const char* toString(SdfType t)
{
    switch (t)
    {
        case SdfType::SPHERE: return "Sphere";
        case SdfType::BOX: return "Box";
        case SdfType::CAPSULE: return "Capsule";
        case SdfType::HEIGHTMAP: return "Height Map";
        case SdfType::OCTREE_DIFFERENCE: return "Octree Difference";
        case SdfType::OCTAHEDRON: return "Octahedron";
        case SdfType::PYRAMID: return "Pyramid";
        default: return "Unknown";
    }
}
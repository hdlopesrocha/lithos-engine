#include "SDF.hpp"

glm::vec3 SDF::getPosition(float * sdf, const BoundingCube &cube) {
    // Early exit if there's no surface inside this cube
    SpaceType eval = SDF::eval(sdf);
    if(eval != SpaceType::Surface) {
        return cube.getCenter();  // or some fallback value
    }
    glm::vec3 normals[8];
    for (int i = 0; i < 8; ++i) {
        normals[i] = SDF::getNormalFromPosition(sdf, cube, cube.getCorner(i));
    }

    glm::mat3 ATA(0.0f);
    glm::vec3 ATb(0.0f);

    for (int i = 0; i < 12; ++i) {
        glm::ivec2 edge = SDF_EDGES[i];
        float d0 = sdf[edge[0]];
        float d1 = sdf[edge[1]];

		bool sign0 = d0 < 0.0f;
		bool sign1 = d1 < 0.0f;

        if (sign0 != sign1) {
            glm::vec3 p0 = cube.getCorner(edge[0]);
            glm::vec3 p1 = cube.getCorner(edge[1]);
            float denom = d0 - d1;
            float t = (denom != 0.0f) ? glm::clamp(d0 / denom, 0.0f, 1.0f) : 0.5f;
            glm::vec3 p = glm::mix(p0, p1, t);
            glm::vec3 n = glm::normalize(glm::mix(normals[edge[0]], normals[edge[1]], t));

            float d = glm::dot(n, p);
            ATA += glm::outerProduct(n, n);
            ATb += n * d;
        }
    }

    if (glm::determinant(ATA) > 1e-5f) {
        return Math::solveLinearSystem(ATA, ATb);
    } else {
        return getAveragePosition(sdf, cube); // e.g., average of surface crossings
    }
}

glm::vec3 SDF::getAveragePosition(float *sdf, const BoundingCube &cube) {
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

glm::vec3 SDF::getAveragePosition2(float * sdf, const BoundingCube &cube) {
    glm::vec3 avg = getAveragePosition(sdf, cube);
    glm::vec3 normal = getNormalFromPosition(sdf, cube, avg);
    float d = interpolate(sdf, avg, cube);
    return avg - normal * d;
}

glm::vec3 SDF::getNormal(float * sdf, const BoundingCube& cube) {
    const float dx = cube.getLengthX(); // or half size if your sdf spacing is half
    const float inv2dx = 1.0f / (2.0f * dx);

    // Gradient approximation via central differences:
    float gx = (sdf[1] + sdf[5] + sdf[3] + sdf[7] - sdf[0] - sdf[4] - sdf[2] - sdf[6]) * 0.25f;
    float gy = (sdf[2] + sdf[3] + sdf[6] + sdf[7] - sdf[0] - sdf[1] - sdf[4] - sdf[5]) * 0.25f;
    float gz = (sdf[4] + sdf[5] + sdf[6] + sdf[7] - sdf[0] - sdf[1] - sdf[2] - sdf[3]) * 0.25f;

    glm::vec3 normal(gx, gy, gz);
    return glm::normalize(normal * inv2dx);
}

glm::vec3 SDF::getNormalFromPosition(float * sdf, const BoundingCube& cube, const glm::vec3& position) {
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


float SDF::opUnion(float d1, float d2) {
    return glm::min(d1,d2);
}

float SDF::opSubtraction(float d1, float d2) {
    return glm::max(d1,-d2);
}

float SDF::opIntersection(float d1, float d2) {
    return glm::max(d1,d2);
}

float SDF::opXor(float d1, float d2) {
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



// distance to segment AB
inline float sdSegment(const glm::vec3 &p, const glm::vec3 &a, const glm::vec3 &b) {
    glm::vec3 pa = p - a;
    glm::vec3 ba = b - a;
    float t = glm::clamp(glm::dot(pa, ba) / glm::dot(ba, ba), 0.0f, 1.0f);
    return glm::length(pa - ba * t);
}

// exact distance to triangle ABC (Euclidean)
inline float sdTriangle(const glm::vec3 &p, const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c) {
    // plane normal
    glm::vec3 ab = b - a;
    glm::vec3 ac = c - a;
    glm::vec3 n = glm::cross(ab, ac);
    float nlen2 = glm::dot(n, n);
    if (nlen2 < 1e-12f) {
        // degenerate triangle -> fallback to segment distances
        return std::min({ sdSegment(p, a, b), sdSegment(p, b, c), sdSegment(p, c, a) });
    }
    glm::vec3 nn = n / glm::sqrt(nlen2);
    float distPlane = glm::dot(p - a, nn);
    glm::vec3 proj = p - distPlane * nn;

    // inside-triangle test (same-side)
    glm::vec3 ap = proj - a;
    glm::vec3 bp = proj - b;
    glm::vec3 cp = proj - c;
    if (glm::dot(glm::cross(ab, ap), n) >= 0.0f &&
        glm::dot(glm::cross(c - b, bp), n) >= 0.0f &&
        glm::dot(glm::cross(a - c, cp), n) >= 0.0f) {
        return std::abs(distPlane); // perpendicular hits inside triangle
    }

    // otherwise distance to triangle edges
    return std::min({ sdSegment(p, a, b), sdSegment(p, b, c), sdSegment(p, c, a) });
}

// Apply model (4x4) to vec3
inline glm::vec3 transformPos(const glm::mat4 &M, const glm::vec3 &v) {
    glm::vec4 t = M * glm::vec4(v, 1.0f);
    return glm::vec3(t) / t.w;
}

// Exact SDF for an affine-transformed pyramid (square base)
// - p: query point in world space
// - h: original pyramid height (apex at (0,h,0), base at y=0)
// - a: half base size (base corners at +/-a, y=0)
// - model: 4x4 model matrix (scaling, rotation, translation). If you only want non-uniform scale use glm::scale(...)
float SDF::pyramid(const glm::vec3 &p, float h, float a, const glm::mat4 &model) {
    // original model-space vertices
    glm::vec3 apex(0.0f, h, 0.0f);
    glm::vec3 v0(-a, 0.0f, -a);
    glm::vec3 v1( a, 0.0f, -a);
    glm::vec3 v2( a, 0.0f,  a);
    glm::vec3 v3(-a, 0.0f,  a);

    // transform to world space
    glm::vec3 A = transformPos(model, apex);
    glm::vec3 V0 = transformPos(model, v0);
    glm::vec3 V1 = transformPos(model, v1);
    glm::vec3 V2 = transformPos(model, v2);
    glm::vec3 V3 = transformPos(model, v3);

    // centroid (a point guaranteed inside the (convex) pyramid)
    glm::vec3 centroid = (A + V0 + V1 + V2 + V3) * (1.0f / 5.0f);

    // distances to 4 side triangles (A, Vi, Vi+1)
    float d0 = sdTriangle(p, A, V0, V1);
    float d1 = sdTriangle(p, A, V1, V2);
    float d2 = sdTriangle(p, A, V2, V3);
    float d3 = sdTriangle(p, A, V3, V0);

    // base as two triangles (V0,V1,V2) and (V2,V3,V0)
    float db0 = sdTriangle(p, V0, V1, V2);
    float db1 = sdTriangle(p, V2, V3, V0);

    float dist = std::min({ d0, d1, d2, d3, db0, db1 });

    // SIGN: determine if p is inside the convex polyhedron.
    // For each face compute outward-pointing normal and test dot(p - a, normal) <= 0.
    auto faceOutward = [&](const glm::vec3 &a_, const glm::vec3 &b_, const glm::vec3 &c_) {
        glm::vec3 n = glm::normalize(glm::cross(b_ - a_, c_ - a_));
        // If normal points towards centroid, flip it so normals point outward
        if (glm::dot(n, centroid - a_) > 0.0f) n = -n;
        return n;
    };

    glm::vec3 n0 = faceOutward(A, V0, V1);
    glm::vec3 n1 = faceOutward(A, V1, V2);
    glm::vec3 n2 = faceOutward(A, V2, V3);
    glm::vec3 n3 = faceOutward(A, V3, V0);
    glm::vec3 nb0 = faceOutward(V0, V1, V2); // base tri normals (should point downward/outward)
    glm::vec3 nb1 = faceOutward(V2, V3, V0);

    bool inside =
        (glm::dot(p - A, n0) <= 0.0f) &&
        (glm::dot(p - A, n1) <= 0.0f) &&
        (glm::dot(p - A, n2) <= 0.0f) &&
        (glm::dot(p - A, n3) <= 0.0f) &&
        (glm::dot(p - V0, nb0) <= 0.0f) &&
        (glm::dot(p - V2, nb1) <= 0.0f);

    return inside ? -dist : dist;
}

float SDF::opSmoothUnion(float d1, float d2, float k) {
    float h = glm::clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return glm::mix( d2, d1, h ) - k*h*(1.0-h);
}

float SDF::opSmoothSubtraction(float d1, float d2, float k) {
    float h = glm::clamp( 0.5 - 0.5*(d2+d1)/k, 0.0, 1.0 );
    return glm::mix( d1, -d2, h ) + k*h*(1.0-h);
}

float SDF::opSmoothIntersection(float d1, float d2, float k) {
    float h = glm::clamp( 0.5 - 0.5*(d1-d2)/k, 0.0, 1.0 );
    return glm::mix( d1, d2, h ) + k*h*(1.0-h);
}

float SDF::interpolate(float * sdf, glm::vec3 position, BoundingCube cube) {
    glm::vec3 local = (position - cube.getMin()) / cube.getLength(); // [0,1]^3
    float x = local.x, y = local.y, z = local.z;

    // Interpolate along z for each (x, y) pair
    float v000 = glm::mix(sdf[0], sdf[1], z); // (0,0,0)-(0,0,1)
    float v010 = glm::mix(sdf[2], sdf[3], z); // (0,1,0)-(0,1,1)
    float v100 = glm::mix(sdf[4], sdf[5], z); // (1,0,0)-(1,0,1)
    float v110 = glm::mix(sdf[6], sdf[7], z); // (1,1,0)-(1,1,1)

    // Interpolate along y
    float v00 = glm::mix(v000, v010, y); // (0,*,*)
    float v10 = glm::mix(v100, v110, y); // (1,*,*)

    // Interpolate along x
    return glm::mix(v00, v10, x);
}

void SDF::getChildSDF(float * sdf, int i , float * result) {
    BoundingCube canonicalCube = BoundingCube(glm::vec3(0.0f), 1.0f);
    BoundingCube cube = canonicalCube.getChild(i);
    for (int j = 0; j < 8; ++j) {
        result[j] = interpolate(sdf, cube.getCorner(j), canonicalCube);
    }
}

void SDF::copySDF(float * src, float * dst) {
    for (int corner = 0; corner < 8; ++corner) {
        dst[corner] = src[corner];
    }
}

BoxDistanceFunction::BoxDistanceFunction(glm::vec3 center, glm::vec3 length): center(center), length(length) {

}

float BoxDistanceFunction::distance(const glm::vec3 p, Transformation model) {
    glm::vec3 pos = p - getCenter(model); // Move point into model space
    pos = glm::inverse(model.quaternion) * pos;
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

float SphereDistanceFunction::distance(const glm::vec3 p, Transformation model) {
    glm::vec3 pos = p - getCenter(model); // Move point into model space
    pos = glm::inverse(model.quaternion) * pos;
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

float CapsuleDistanceFunction::distance(const glm::vec3 p, Transformation model) {
    glm::vec3 pos = p - model.translate; // Move point into model space
    pos = glm::inverse(model.quaternion) * pos;
    return SDF::capsule(pos/model.scale, a, b, radius);
}

SdfType CapsuleDistanceFunction::getType() const {
    return SdfType::CAPSULE;
}

glm::vec3 CapsuleDistanceFunction::getCenter(Transformation model) const {
    return 0.5f*(this->a+this->b)+model.translate;
}

HeightMapDistanceFunction::HeightMapDistanceFunction(HeightMap * map) {
    this->map = map;
}

float HeightMapDistanceFunction::distance(const glm::vec3 p, Transformation model)  {
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

glm::vec3 HeightMapDistanceFunction::getCenter(Transformation model) const {
    return this->map->getCenter();
}

OctahedronDistanceFunction::OctahedronDistanceFunction(glm::vec3 center, float radius): center(center), radius(radius) {
    
}

float OctahedronDistanceFunction::distance(const glm::vec3 p, Transformation model) {
    glm::vec3 pos = p - getCenter(model); // Move point into model space
    pos = glm::inverse(model.quaternion) * pos;
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

float PyramidDistanceFunction::distance(const glm::vec3 p, Transformation model)  {
    glm::vec3 pos = p - getCenter(model); // Move point into model space
    pos = glm::inverse(model.quaternion) * pos;
    return SDF::pyramid(pos, height, 0.5f, glm::scale(glm::mat4(1.0f), model.scale));
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
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

float SDF::cylinder(glm::vec3 p, float r, float h) {
    glm::vec2 d = glm::vec2(glm::length(glm::vec2(p.x, p.z)) - r, glm::abs(p.y) - h);
    return glm::min(glm::max(d.x, d.y), 0.0f) + glm::length(glm::max(d, 0.0f));
}

float SDF::capsule(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r ) {
    glm::vec3 pa = p - a, ba = b - a;
    float h = glm::clamp( glm::dot(pa,ba)/glm::dot(ba,ba), 0.0f, 1.0f );
    return glm::length( pa - ba*h ) - r;
}

float SDF::torus(glm::vec3 p, glm::vec2 t ) {
  glm::vec2 q = glm::vec2(glm::length(glm::vec2(p.x,p.z))-t.x,p.y);
  return glm::length(q)-t.y;
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

// Deterministic pseudo-random [0,1] from integer coords
inline float randFromPerlin(const glm::ivec3& cell, float seed = 0.0f) {
    float n = stb_perlin_noise3(
        cell.x * 0.123f,
        cell.y * 0.456f,
        cell.z * 0.789f + seed,0,0,0
    );
    return 0.5f * (n + 1.0f); // map [-1,1] -> [0,1]
}

// Safe Voronoi 3D with adjustable cell size
float SDF::voronoi3D(const glm::vec3& p, float cellSize = 1.0f, float seed = 0.0f) {
    if (cellSize <= 0.0f) {
        return 0.0f; // safe fallback
    }

    // Work in normalized lattice space
    glm::vec3 q = p / cellSize;

    // Domain warp to break grid artifacts
    q += 0.3f * glm::vec3(
        stb_perlin_noise3(q.x, q.y, q.z, 0,0,0),
        stb_perlin_noise3(q.y, q.z, q.x, 0,0,0),
        stb_perlin_noise3(q.z, q.x, q.y, 0,0,0)
    );

    glm::ivec3 baseCell = glm::floor(q);
    float minDist = 1e10f;

    // Check neighbors (27 cells)
    for (int k = -1; k <= 1; k++) {
        for (int j = -1; j <= 1; j++) {
            for (int i = -1; i <= 1; i++) {
                glm::ivec3 neighbor = baseCell + glm::ivec3(i, j, k);

                float ox = randFromPerlin(neighbor, seed);
                float oy = randFromPerlin(neighbor + glm::ivec3(7, 3, 5), seed + 17.0f);
                float oz = randFromPerlin(neighbor + glm::ivec3(11, 19, 23), seed + 37.0f);

                glm::vec3 cellSeed = glm::vec3(neighbor) + glm::vec3(ox, oy, oz);

                float d = glm::length(q - cellSeed);
                minDist = glm::min(minDist, d);
            }
        }
    }

    // Normalize: [0, sqrt(3)] → [0,1]
    return minDist / sqrtf(3.0f);
}


glm::vec3 faceOutward(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, const glm::vec3 &centroid) {
    // normal da face
    glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));

    // se a normal aponta para dentro, inverter
    if (glm::dot(n, centroid - a) > 0.0f)
        n = -n;

    return n;
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
float SDF::pyramid(const glm::vec3 &p, float h, float a) {
      // Vértices locais
    glm::vec3 apex(0.0f, h, 0.0f);
    glm::vec3 v0(-a, 0.0f, -a);
    glm::vec3 v1( a, 0.0f, -a);
    glm::vec3 v2( a, 0.0f,  a);
    glm::vec3 v3(-a, 0.0f,  a);

    // mesmo cálculo de distâncias (sem model matrix!)
    glm::vec3 centroid = (apex + v0 + v1 + v2 + v3) / 5.0f;

    float d0 = sdTriangle(p, apex, v0, v1);
    float d1 = sdTriangle(p, apex, v1, v2);
    float d2 = sdTriangle(p, apex, v2, v3);
    float d3 = sdTriangle(p, apex, v3, v0);
    float db0 = sdTriangle(p, v0, v1, v2);
    float db1 = sdTriangle(p, v2, v3, v0);

    float dist = std::min({d0, d1, d2, d3, db0, db1});

    auto faceOutward = [&](const glm::vec3 &a_, const glm::vec3 &b_, const glm::vec3 &c_) {
        glm::vec3 n = glm::normalize(glm::cross(b_ - a_, c_ - a_));
        if (glm::dot(n, centroid - a_) > 0.0f) n = -n;
        return n;
    };

    glm::vec3 n0 = faceOutward(apex, v0, v1);
    glm::vec3 n1 = faceOutward(apex, v1, v2);
    glm::vec3 n2 = faceOutward(apex, v2, v3);
    glm::vec3 n3 = faceOutward(apex, v3, v0);
    glm::vec3 nb0 = faceOutward(v0, v1, v2);
    glm::vec3 nb1 = faceOutward(v2, v3, v0);

    bool inside =
        (glm::dot(p - apex, n0) <= 0.0f) &&
        (glm::dot(p - apex, n1) <= 0.0f) &&
        (glm::dot(p - apex, n2) <= 0.0f) &&
        (glm::dot(p - apex, n3) <= 0.0f) &&
        (glm::dot(p - v0, nb0) <= 0.0f) &&
        (glm::dot(p - v2, nb1) <= 0.0f);

    return inside ? -dist : dist;
}


float SDF::cone(glm::vec3 p) {
    // Unit cone: apex at origin, base radius = height = 1
    // q vector derived from Inigo Quilez's formula
    const glm::vec2 q(1.0f, -1.0f);

    // Convert 3D point to 2D (radius in XZ plane, height in Y)
    glm::vec2 w(glm::length(glm::vec2(p.x, p.z)), p.y);

    // Project onto cone sides
    glm::vec2 a = w - q * glm::clamp(glm::dot(w, q) / glm::dot(q, q), 0.0f, 1.0f);
    glm::vec2 b = w - q * glm::vec2(glm::clamp(w.x / q.x, 0.0f, 1.0f), 1.0f);

    // Sign helper
    float k = glm::sign(q.y);
    float d = glm::min(glm::dot(a, a), glm::dot(b, b));
    float s = glm::max(k * (w.x * q.y - w.y * q.x), k * (w.y - q.y));

    return glm::sqrt(d) * glm::sign(s);
}

glm::vec3 SDF::distortPerlin(glm::vec3 p, float amplitude, float frequency) {
    float noiseX = stb_perlin_noise3(p.x*frequency, p.y*frequency, p.z*frequency, 0, 0, 0);
    float noiseY = stb_perlin_noise3((p.x+100)*frequency, (p.y+100)*frequency, (p.z+100)*frequency, 0, 0, 0);
    float noiseZ = stb_perlin_noise3((p.x+200)*frequency, (p.y+200)*frequency, (p.z+200)*frequency, 0, 0, 0);
    return p + amplitude * glm::vec3(noiseX, noiseY, noiseZ);
}

glm::vec3 SDF::distortPerlinFractal(glm::vec3 p, float frequency, int octaves, float lacunarity = 2.0f, float gain = 0.5f) {
    glm::vec3 totalNoise(0.0f);
    float freq = frequency;
    float amp = 1.0f;

    // For each axis, use different offsets to decorrelate noise
    glm::vec3 offsetX = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 offsetY = glm::vec3(100.0f, 100.0f, 100.0f);
    glm::vec3 offsetZ = glm::vec3(200.0f, 200.0f, 200.0f);

    for (int i = 0; i < octaves; ++i) {
        float nx = stb_perlin_noise3(
            p.x * freq + offsetX.x,
            p.y * freq + offsetX.y,
            p.z * freq + offsetX.z,
            0,0,0
        );
        float ny = stb_perlin_noise3(
            p.x * freq + offsetY.x,
            p.y * freq + offsetY.y,
            p.z * freq + offsetY.z,
            0,0,0
        );
        float nz = stb_perlin_noise3(
            p.x * freq + offsetZ.x,
            p.y * freq + offsetZ.y,
            p.z * freq + offsetZ.z,
            0,0,0
        );

        totalNoise += amp * glm::vec3(nx, ny, nz);

        freq *= lacunarity; // increase frequency
        amp *= gain;        // decrease amplitude
    }

    return totalNoise;
}

float SDF::distortedCarveFractalSDF(glm::vec3 p, 
                                    float threshold, 
                                    float frequency, 
                                    int octaves = 4, 
                                    float lacunarity = 2.0f, 
                                    float gain = 0.5f) {

    float noiseValue = 0.0f;
    float freq = frequency;
    float amp = 1.0f;
    float d = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        noiseValue += amp * stb_perlin_noise3(
            p.x * freq, 
            p.y * freq, 
            p.z * freq, 
            0, 0, 0
        );
        freq *= lacunarity;
        amp *= gain;
    }

    if (noiseValue > threshold) {
        d += (noiseValue - threshold);
    }

    return d;
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
        result[j] = sdf[j] == INFINITY ? INFINITY : interpolate(sdf, cube.getCorner(j), canonicalCube);
    }
}

void SDF::copySDF(const float * src, float * dst) {
    for (int corner = 0; corner < 8; ++corner) {
        dst[corner] = src == NULL ? INFINITY : src[corner];
    }
}

SpaceType SDF::eval(float * sdf) {
    bool hasPositive = false;
    bool hasNegative = false;
    for (int i = 0; i < 8; ++i) {  
        if(sdf[i] == INFINITY) {
            return SpaceType::Empty;
        } else if (sdf[i] >= 0.0f) {
            hasPositive = true;
        } else {
            hasNegative = true;
        }
    }
    return hasNegative && hasPositive ? SpaceType::Surface : (hasPositive ? SpaceType::Empty : SpaceType::Solid);
}

#include "math.hpp"

namespace {
    Vertex getVertex(glm::vec3 v, glm::vec3 n, glm::vec2 t) {
        return Vertex(v, n, t, 0);
    }
}

BoxGeometry::BoxGeometry(const BoundingBox &box) : Geometry(true) {
    glm::vec3 min = box.getMin();
    glm::vec3 max = box.getMax();

    // Cube corners (right-handed system)
    glm::vec3 corners[8] = {
        {min.x, min.y, min.z}, // 0
        {max.x, min.y, min.z}, // 1
        {max.x, max.y, min.z}, // 2
        {min.x, max.y, min.z}, // 3
        {min.x, min.y, max.z}, // 4
        {max.x, min.y, max.z}, // 5
        {max.x, max.y, max.z}, // 6
        {min.x, max.y, max.z}  // 7
    };

    // UV maps per face (corrected orientation)
    static glm::vec2 uvFront[4]  = {{0,0}, {1,0}, {1,1}, {0,1}};
    static glm::vec2 uvBack[4]   = {{1,0}, {0,0}, {0,1}, {1,1}};
    static glm::vec2 uvLeft[4]   = {{1,0}, {0,0}, {0,1}, {1,1}};
    static glm::vec2 uvRight[4]  = {{0,0}, {1,0}, {1,1}, {0,1}};
    static glm::vec2 uvTop[4]    = {{0,1}, {1,1}, {1,0}, {0,0}};
    static glm::vec2 uvBottom[4] = {{0,0}, {1,0}, {1,1}, {0,1}};

    struct Face {
        int a, b, c, d;
        glm::vec3 n;
        glm::vec2* uv;
    };

    Face faces[6] = {
        {4, 5, 6, 7, { 0,  0,  1}, uvFront},   // front (+Z)
        {1, 0, 3, 2, { 0,  0, -1}, uvBack},    // back (-Z)
        {0, 4, 7, 3, {-1,  0,  0}, uvLeft},    // left (-X)
        {5, 1, 2, 6, { 1,  0,  0}, uvRight},   // right (+X)
        {3, 7, 6, 2, { 0,  1,  0}, uvTop},     // top (+Y)
        {0, 1, 5, 4, { 0, -1,  0}, uvBottom}   // bottom (-Y)
    };

    for (auto &f : faces) {
        addVertex(getVertex(corners[f.a], f.n, f.uv[0]));
        addVertex(getVertex(corners[f.b], f.n, f.uv[1]));
        addVertex(getVertex(corners[f.c], f.n, f.uv[2]));

        addVertex(getVertex(corners[f.a], f.n, f.uv[0]));
        addVertex(getVertex(corners[f.c], f.n, f.uv[2]));
        addVertex(getVertex(corners[f.d], f.n, f.uv[3]));
    }
}
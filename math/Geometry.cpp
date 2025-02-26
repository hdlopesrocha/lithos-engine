#include "math.hpp"

Geometry::Geometry(/* args */) {
}

Geometry::~Geometry() {
}


Vertex * Geometry::addVertex(Vertex vertex, bool textureUnique){
    std::string key = textureUnique ? vertex.toString() : vertex.toKey();
    if(!compactMap.count(key)) {
        compactMap[key] = compactMap.size();
        vertices.push_back(vertex); 
    }
    int idx = compactMap[key];
    indices.push_back(idx);
    return &(vertices[idx]);
}

glm::vec3 Geometry::getNormal(Vertex * a, Vertex * b, Vertex * c) {
    glm::vec3 v1 = b->position-a->position;
    glm::vec3 v2 = c->position-a->position;
    return glm::normalize(glm::cross(v1 ,v2));
}

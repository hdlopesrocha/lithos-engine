#include "math.hpp"

Geometry::Geometry(/* args */) {
}

Geometry::~Geometry() {
}



Vertex * Geometry::addVertex(Vertex vertex){
    auto it = compactMap.find(vertex);
    int idx = 0;

    if (it != compactMap.end()) {
        idx = it->second;
    } else {
        idx = compactMap.size();
        compactMap.insert({vertex, idx});
        vertices.push_back(vertex); 
    }
    indices.push_back(idx);
    return &(vertices[idx]);
}

glm::vec3 Geometry::getNormal(Vertex * a, Vertex * b, Vertex * c) {
    glm::vec3 v1 = b->position-a->position;
    glm::vec3 v2 = c->position-a->position;
    return glm::normalize(glm::cross(v1 ,v2));
}

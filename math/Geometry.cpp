#include "math.hpp"

Geometry::Geometry(/* args */) {
    this->center = glm::vec3(0);
}

Geometry::~Geometry() {
}

glm::vec3 Geometry::getCenter(){
    return center;
}

void Geometry::setCenter(){
    if(vertices.size()){
        glm::vec3 min = vertices[0].position;
        glm::vec3 max = vertices[0].position;

        for(Vertex vertex : vertices) {
            min = glm::min(min, vertex.position);
            max = glm::min(max, vertex.position);
        }
        center = (min+max)*0.5f;
    }
    else {
        center = glm::vec3(0);
    }
}

Vertex * Geometry::addVertex(const Vertex &vertex) {
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

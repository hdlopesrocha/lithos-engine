#include "math.hpp"

Geometry::Geometry(bool reusable) {
    this->center = glm::vec3(0);
    this->reusable = reusable;
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
            glm::vec3 pos = vertex.position;
            min = glm::min(min, pos);
            max = glm::min(max, pos);
        }
        center = (min+max)*0.5f;
    }
    else {
        center = glm::vec3(0);
    }
}

void Geometry::addTriangle(const Vertex &v0, const Vertex &v1, const Vertex &v2) {
    addVertex(v0);
    addVertex(v1);
    addVertex(v2);
}

void Geometry::addVertex(const Vertex &vertex) {
    auto [it, inserted] = compactMap.try_emplace(vertex, compactMap.size());
    size_t idx = it->second;
    
    if (inserted) {
        vertices.push_back(vertex);
    }
    indices.push_back(idx);
}

glm::vec3 Geometry::getNormal(Vertex * a, Vertex * b, Vertex * c) {
    glm::vec3 v1 = b->position-a->position;
    glm::vec3 v2 = c->position-a->position;
    return glm::normalize(glm::cross(v1 ,v2));
}

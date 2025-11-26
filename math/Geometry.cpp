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

bool Geometry::addTriangle(const Vertex &v0, const Vertex &v1, const Vertex &v2) {
    uint idx0 = getIndex(v0);
    uint idx1 = getIndex(v1);
    uint idx2 = getIndex(v2);
    if(triangleSet.find(Triple<uint,uint,uint>(idx0, idx1, idx2)) == triangleSet.end()) {
        idx0 = idx0 == UINT_MAX ? addVertex(v0) : addVertex(idx0);
        idx1 = idx1 == UINT_MAX ? addVertex(v1) : addVertex(idx1);
        idx2 = idx2 == UINT_MAX ? addVertex(v2) : addVertex(idx2);
        triangleSet.insert(Triple<uint,uint,uint>(idx0, idx1, idx2));
        return true;
    } else {
        //std::cout << "Duplicate triangle detected and ignored." << std::endl;
        return false;
    }
}

uint Geometry::getIndex(const Vertex &vertex) {
    auto it = compactMap.find(vertex);
    if (it != compactMap.end()) {
        return it->second;
    }
    return UINT_MAX; // or some other invalid value
}

uint Geometry::addVertex(const Vertex &vertex) {
    auto [it, inserted] = compactMap.try_emplace(vertex, compactMap.size());
    size_t idx = it->second;
    
    if (inserted) {
        vertices.push_back(vertex);
    }
    indices.push_back(idx);
    return idx;
}

uint Geometry::addVertex(const uint idx){
    indices.push_back(idx);
    return idx;
}

glm::vec3 Geometry::getNormal(Vertex * a, Vertex * b, Vertex * c) {
    glm::vec3 v1 = b->position-a->position;
    glm::vec3 v2 = c->position-a->position;
    return glm::normalize(glm::cross(v1 ,v2));
}

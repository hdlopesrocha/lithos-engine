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

void Geometry::normalize() {
    for(int i=0; i < vertices.size() ; ++i) {
        Vertex v = vertices[i];
        v.normal = glm::normalize(v.normal);
        vertices[i] = v;
    }
}

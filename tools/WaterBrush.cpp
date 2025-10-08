#include "tools.hpp"

WaterBrush::WaterBrush(int water){
    this->water = water;
}

int WaterBrush::paint(const Vertex &vertex) const {    
    int brushIndex;
    glm::vec3 n = glm::normalize(vertex.normal);
    if (glm::dot(glm::vec3(0.0f,1.0f,0.0f), n ) < 0.5 ){
        brushIndex= water;
    } else {
        brushIndex = water;
    } 
    return brushIndex;
}


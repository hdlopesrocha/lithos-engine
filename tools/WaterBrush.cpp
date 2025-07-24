#include "tools.hpp"

WaterBrush::WaterBrush(int water){
    this->water = water;
}

void WaterBrush::paint(Vertex &vertex) const {    
    uint brushIndex;
    glm::vec3 n = glm::normalize(vertex.normal);
    if (glm::dot(glm::vec3(0.0f,1.0f,0.0f), n ) < 0.5 ){
        brushIndex= DISCARD_BRUSH_INDEX;
    } else {
        brushIndex = water;
    } 
    vertex.brushIndex = brushIndex;
}


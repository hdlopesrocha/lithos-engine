#include "tools.hpp"

WaterBrush::WaterBrush(Brush* water){
    this->water = water;
}

void WaterBrush::paint(Vertex &vertex) const {
    float steepness =glm::dot(glm::vec3(0.0f,1.0f,0.0f), vertex.normal );
    
    uint brushIndex;
    if (glm::dot(glm::vec3(0.0f,1.0f,0.0f), vertex.normal ) <=0 ){
        brushIndex= DISCARD_BRUSH_INDEX;
    } else {
        brushIndex = water->brushIndex;
    } 
    vertex.brushIndex = brushIndex;
}


#include "tools.hpp"

WaterBrush::WaterBrush(Texture* water){
    discardTexture.index = DISCARD_BRUSH_INDEX;
    this->water = water;
}

void WaterBrush::paint(Vertex * vertex) {
    float steepness =glm::dot(glm::vec3(0.0f,1.0f,0.0f), vertex->normal );
    
    Texture * texture;
    if (glm::dot(glm::vec3(0.0f,1.0f,0.0f), vertex->normal ) <=0 ){
        texture= &discardTexture;
    } else {
        texture = water;
    } 
    vertex->brushIndex = texture->index;
}


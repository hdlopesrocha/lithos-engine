#include "tools.hpp"

LandBrush::LandBrush(std::vector<Brush*> brushes){
    this->underground = brushes[7];
    this->grass = brushes[2];
    this->sand = brushes[3];
    this->softSand = brushes[14];
    this->rock = brushes[4];
    this->snow = brushes[5];
    this->grassMixSand = brushes[9];
    this->grassMixSnow = brushes[10];
    this->rockMixGrass = brushes[11];
    this->rockMixSnow = brushes[12];
    this->rockMixSand = brushes[13];
}

void LandBrush::paint(Vertex * vertex) const {
    float steepness =glm::dot(glm::vec3(0.0f,1.0f,0.0f), vertex->normal );
    int grassLevel = 25;
    int sandLevel = 5;

    uint brushIndex;
    if (glm::dot(glm::vec3(0.0f,1.0f,0.0f), vertex->normal ) <=0 ){
        brushIndex = DISCARD_BRUSH_INDEX;
    } else if(steepness < 0.8 ){
        brushIndex = rock->brushIndex;
    } else if(steepness < 0.9 ){
        if(vertex->position.y < sandLevel -1){
            brushIndex = rock->brushIndex;
        } else if(vertex->position.y < sandLevel){
            brushIndex = rockMixSand->brushIndex;
        } else if(vertex->position.y < grassLevel){
            brushIndex = rockMixGrass->brushIndex;
        } else {
            brushIndex = rockMixSnow->brushIndex;
        }
    } else if(vertex->position.y < sandLevel-1){
        brushIndex = softSand->brushIndex;
    } else if(vertex->position.y < sandLevel){
        brushIndex = sand->brushIndex;
    } else if(vertex->position.y < sandLevel+1){
        brushIndex = grassMixSand->brushIndex;
    } else if(vertex->position.y < grassLevel){
        brushIndex = grass->brushIndex;
    } else if(vertex->position.y < grassLevel+1){
        brushIndex = grassMixSnow->brushIndex;
    } else {
        brushIndex = snow->brushIndex;
    }

    vertex->brushIndex = brushIndex;
}

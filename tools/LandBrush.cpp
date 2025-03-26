#include "tools.hpp"

LandBrush::LandBrush(){
    this->underground = 7;
    this->grass = 2;
    this->sand = 3;
    this->softSand = 14;
    this->rock = 4;
    this->snow = 5;
    this->grassMixSand = 9;
    this->grassMixSnow = 10;
    this->rockMixGrass = 11;
    this->rockMixSnow = 12;
    this->rockMixSand = 13;
}

void LandBrush::paint(Vertex &vertex) const {
    float steepness =glm::dot(glm::vec3(0.0f,1.0f,0.0f), vertex.normal );
    int grassLevel = 25;
    int sandLevel = 2;

    uint brushIndex;
    if (glm::dot(glm::vec3(0.0f,1.0f,0.0f), vertex.normal ) <=0 ){
        brushIndex = DISCARD_BRUSH_INDEX;
    } else if(steepness < 0.8 ){
        brushIndex = rock;
    } else if(steepness < 0.9 ){
        if(vertex.position.y < sandLevel -1){
            brushIndex = rock;
        } else if(vertex.position.y < sandLevel){
            brushIndex = rockMixSand;
        } else if(vertex.position.y < grassLevel){
            brushIndex = rockMixGrass;
        } else {
            brushIndex = rockMixSnow;
        }
    } else if(vertex.position.y < sandLevel-1){
        brushIndex = softSand;
    } else if(vertex.position.y < sandLevel){
        brushIndex = sand;
    } else if(vertex.position.y < sandLevel+1){
        brushIndex = grassMixSand;
    } else if(vertex.position.y < grassLevel){
        brushIndex = grass;
    } else if(vertex.position.y < grassLevel+1){
        brushIndex = grassMixSnow;
    } else {
        brushIndex = snow;
    }

    vertex.brushIndex = brushIndex;
}

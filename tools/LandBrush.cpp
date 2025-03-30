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
    int grassLevel = 256;
    int sandLevel = 60;
    int softSandLevel = 54;
    uint brushIndex;
    if (glm::dot(glm::vec3(0.0f,1.0f,0.0f), vertex.normal ) <=0 ){
        brushIndex = DISCARD_BRUSH_INDEX;
    } else if(steepness < 0.980 ){
        brushIndex = rock;
    } else if(steepness < 0.985 ){
        if(vertex.position.y < softSandLevel){
            brushIndex = rock;
        } else if(vertex.position.y < sandLevel){
            brushIndex = rockMixSand;
        } else if(vertex.position.y < grassLevel){
            brushIndex = rockMixGrass;
        } else {
            brushIndex = rockMixSnow;
        }
    } else if(vertex.position.y < softSandLevel && steepness > 0.99925){
        brushIndex = softSand;
    } else if(vertex.position.y < sandLevel && steepness>0.99){
        brushIndex = sand;
    } else if(vertex.position.y < sandLevel+4 && steepness>0.98){
        brushIndex = grassMixSand;
    } else if(vertex.position.y < grassLevel){
        brushIndex = grass;
    } else if(vertex.position.y < grassLevel+32){
        brushIndex = grassMixSnow;
    } else {
        brushIndex = snow;
    }

    vertex.brushIndex = brushIndex;
}

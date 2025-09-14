#include "tools.hpp"

LandBrush::LandBrush(){
    this->underground = 8;
    this->grass = 3;
    this->sand = 4;
    this->softSand = 15;
    this->rock = 5;
    this->snow = 6;
    this->grassMixSand = 10;
    this->grassMixSnow = 11;
    this->rockMixGrass = 12;
    this->rockMixSnow = 13;
    this->rockMixSand = 14;
}

void LandBrush::paint(Vertex &vertex) const {
    glm::vec3 n = glm::normalize(vertex.normal);
    float steepness =glm::dot(glm::vec3(0.0f,1.0f,0.0f), n );
    int grassLevel = 256;
    int sandLevel = 16;
    int softSandLevel = 2;
    uint brushIndex;
    if (glm::dot(glm::vec3(0.0f,1.0f,0.0f), n ) <=0 ){
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
    } else if(vertex.position.y < softSandLevel){
        brushIndex = softSand;
    } else if(vertex.position.y < sandLevel){
        brushIndex = sand;
    } else if(vertex.position.y < sandLevel+4){
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

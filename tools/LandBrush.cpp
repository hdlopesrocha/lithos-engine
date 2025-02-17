#include "tools.hpp"

LandBrush::LandBrush(std::vector<Brush*> brushes){
    this->underground = brushes[7]->textureIndex;
    this->grass = brushes[2]->textureIndex;
    this->sand = brushes[3]->textureIndex;
    this->softSand = brushes[15]->textureIndex;
    this->rock = brushes[4]->textureIndex;
    this->snow = brushes[5]->textureIndex;
    this->grassMixSand = brushes[9]->textureIndex;
    this->grassMixSnow = brushes[10]->textureIndex;
    this->rockMixGrass = brushes[11]->textureIndex;
    this->rockMixSnow = brushes[12]->textureIndex;
    this->rockMixSand = brushes[13]->textureIndex;
}

void LandBrush::paint(Vertex * vertex) {
    float steepness =glm::dot(glm::vec3(0.0f,1.0f,0.0f), vertex->normal );
    int grassLevel = 25;
    int sandLevel = 5;

    uint textureIndex;
    if (glm::dot(glm::vec3(0.0f,1.0f,0.0f), vertex->normal ) <=0 ){
        textureIndex = DISCARD_BRUSH_INDEX;
    } else if(steepness < 0.8 ){
        textureIndex = rock;
    } else if(steepness < 0.9 ){
        if(vertex->position.y < sandLevel -1){
            textureIndex = rock;
        } else if(vertex->position.y < sandLevel){
            textureIndex = rockMixSand;
        } else if(vertex->position.y < grassLevel){
            textureIndex = rockMixGrass;
        } else {
            textureIndex = rockMixSnow;
        }
    } else if(vertex->position.y < sandLevel-1){
        textureIndex = softSand;
    } else if(vertex->position.y < sandLevel){
        textureIndex = sand;
    } else if(vertex->position.y < sandLevel+1){
        textureIndex = grassMixSand;
    } else if(vertex->position.y < grassLevel){
        textureIndex = grass;
    } else if(vertex->position.y < grassLevel+1){
        textureIndex = grassMixSnow;
    } else {
        textureIndex = snow;
    }

    vertex->brushIndex = textureIndex;
}

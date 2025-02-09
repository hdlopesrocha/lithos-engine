#include "tools.hpp"

LandBrush::LandBrush(std::vector<Brush*> brushes){
    discardTexture.index = DISCARD_BRUSH_INDEX;
    this->underground = brushes[7]->texture;
    this->grass = brushes[2]->texture;
    this->sand = brushes[3]->texture;
    this->softSand = brushes[15]->texture;
    this->rock = brushes[4]->texture;
    this->snow = brushes[5]->texture;
    this->grassMixSand = brushes[9]->texture;
    this->grassMixSnow = brushes[10]->texture;
    this->rockMixGrass = brushes[11]->texture;
    this->rockMixSnow = brushes[12]->texture;
    this->rockMixSand = brushes[13]->texture;
}

void LandBrush::paint(Vertex * vertex) {
    float steepness =glm::dot(glm::vec3(0.0f,1.0f,0.0f), vertex->normal );
    int grassLevel = 25;
    int sandLevel = 5;

    Texture * texture;
    if (glm::dot(glm::vec3(0.0f,1.0f,0.0f), vertex->normal ) <=0 ){
        texture= &discardTexture;
    } else if(steepness < 0.8 ){
        texture = rock;
    } else if(steepness < 0.9 ){
        if(vertex->position.y < sandLevel -1){
            texture = rock;
        } else if(vertex->position.y < sandLevel){
            texture = rockMixSand;
        } else if(vertex->position.y < grassLevel){
            texture = rockMixGrass;
        } else {
            texture = rockMixSnow;
        }
    } else if(vertex->position.y < sandLevel-1){
        texture = softSand;
    } else if(vertex->position.y < sandLevel){
        texture = sand;
    } else if(vertex->position.y < sandLevel+1){
        texture = grassMixSand;
    } else if(vertex->position.y < grassLevel){
        texture = grass;
    } else if(vertex->position.y < grassLevel+1){
        texture = grassMixSnow;
    } else {
        texture = snow;
    }

    vertex->texIndex = texture->index;
}

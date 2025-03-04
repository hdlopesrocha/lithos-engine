#include "gl.hpp"

Brush::Brush(uint textureIndex){
    this->textureIndex = textureIndex;
    this->parallaxScale = 0.0;
    this->parallaxMinLayers = 0;
    this->parallaxMaxLayers = 0;
    this->parallaxFade = 0;
    this->parallaxRefine = 0;
    this->shininess = 32;
    this->specularStrength = 0.4;
    this->textureScale = glm::vec2(1.0);
    this->refractiveIndex = 0;
}

Brush::Brush(uint textureIndex, glm::vec2 textureScale,float parallaxScale, float parallaxMinLayers, float parallaxMaxLayers, float parallaxFade, float parallaxRefine, float shininess, float specularStrength, float refractiveIndex){
    this->textureIndex = textureIndex;
    this->parallaxScale = parallaxScale;
    this->parallaxMinLayers = parallaxMinLayers;
    this->parallaxMaxLayers = parallaxMaxLayers;
    this->parallaxFade = parallaxFade;
    this->parallaxRefine = parallaxRefine;
    this->shininess = shininess;
    this->specularStrength = specularStrength;
    this->textureScale = textureScale;
    this->refractiveIndex = refractiveIndex;
}

void Brush::bindBrush(GLuint program, std::string objectName, std::string textureMap, Brush * brush){

    glUniform1ui(glGetUniformLocation(program, (textureMap).c_str()), brush->textureIndex);
    glUniform1f(glGetUniformLocation(program, (objectName +".parallaxScale").c_str() ), brush->parallaxScale);
    glUniform1f(glGetUniformLocation(program, (objectName +".parallaxMinLayers").c_str()), brush->parallaxMinLayers);
    glUniform1f(glGetUniformLocation(program, (objectName +".parallaxMaxLayers").c_str()), brush->parallaxMaxLayers);
    glUniform1f(glGetUniformLocation(program, (objectName +".parallaxFade").c_str()), brush->parallaxFade);
    glUniform1f(glGetUniformLocation(program, (objectName +".parallaxRefine").c_str()), brush->parallaxRefine);
    glUniform1f(glGetUniformLocation(program, (objectName +".shininess").c_str()), brush->shininess);
    glUniform1f(glGetUniformLocation(program, (objectName +".specularStrength").c_str()), brush->specularStrength);
    glUniform1f(glGetUniformLocation(program, (objectName +".refractiveIndex").c_str()), brush->refractiveIndex);
    glUniform2fv(glGetUniformLocation(program, (objectName +".textureScale").c_str()), 1, glm::value_ptr(brush->textureScale));
}

void Brush::bindBrushes(GLuint program, std::string objectName, std::string mapName, std::vector<Brush*> * brushes) {
    glUseProgram(program);

    for(int i = 0; i < brushes->size() ; ++i) {
        Brush * brush = brushes->at(i);
        std::string name = objectName + "[" + std::to_string(i)  +"]";
        brush->brushIndex = i;
        bindBrush(program, name, mapName + "["+std::to_string(i) + "]", brush);
    }
}

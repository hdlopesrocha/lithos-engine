#include "gl.hpp"
#include <glm/gtc/type_ptr.hpp>

Brush::Brush(Texture * texture){
    this->texture = texture;
    this->parallaxScale = 0.2;
    this->parallaxMinLayers = 8;
    this->parallaxMaxLayers = 32;
    this->parallaxFade = 32;
    this->parallaxRefine = 5;
    this->shininess = 32;
    this->specularStrength = 0.4;
    this->textureScale = glm::vec2(1.0);
    this->refractiveIndex = 0;
}

Brush::Brush(Texture * texture, glm::vec2 textureScale,float parallaxScale, float parallaxMinLayers, float parallaxMaxLayers, float parallaxFade, float parallaxRefine, float shininess, float specularStrength, float refractiveIndex){
    this->texture = texture;
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

void Brush::bindBrush(GLuint program, std::string objectName, Brush * brush){
    GLuint parallaxScaleLoc = glGetUniformLocation(program, (objectName +".parallaxScale").c_str() );
    GLuint parallaxMinLayersLoc = glGetUniformLocation(program, (objectName +".parallaxMinLayers").c_str());
    GLuint parallaxMaxLayersLoc = glGetUniformLocation(program, (objectName +".parallaxMaxLayers").c_str());
    GLuint parallaxFadeLoc = glGetUniformLocation(program, (objectName +".parallaxFade").c_str());
    GLuint parallaxRefineLoc = glGetUniformLocation(program, (objectName +".parallaxRefine").c_str());
    GLuint shininessLoc = glGetUniformLocation(program, (objectName +".shininess").c_str());
    GLuint specularStrengthLoc = glGetUniformLocation(program, (objectName +".specularStrength").c_str());
    GLuint textureScaleLoc = glGetUniformLocation(program, (objectName +".textureScale").c_str());
    GLuint refractiveIndexLoc = glGetUniformLocation(program, (objectName +".refractiveIndex").c_str());

    glUniform1f(parallaxScaleLoc, brush->parallaxScale);
    glUniform1f(parallaxMinLayersLoc, brush->parallaxMinLayers);
    glUniform1f(parallaxMaxLayersLoc, brush->parallaxMaxLayers);
    glUniform1f(parallaxFadeLoc, brush->parallaxFade);
    glUniform1f(parallaxRefineLoc, brush->parallaxRefine);
    glUniform1f(shininessLoc, brush->shininess);
    glUniform1f(specularStrengthLoc, brush->specularStrength);
    glUniform1f(refractiveIndexLoc, brush->refractiveIndex);
    glUniform2fv(textureScaleLoc, 1, glm::value_ptr(brush->textureScale));
    
}

void Brush::bindBrushes(GLuint program, std::vector<Brush*> * brushes) {
    glUseProgram(program);

    for(int i = 0; i < brushes->size() ; ++i) {
        Brush * brush = (*brushes)[i];
        std::string objectName = "brushes[" + std::to_string(i)  +"]";
        bindBrush(program, objectName, brush);
    }
}

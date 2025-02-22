#include "tools.hpp"

ProgramData::ProgramData(GLuint program) {
    this->program = program;
    this->modelLoc = glGetUniformLocation(program, "model");
    this->modelViewProjectionLoc = glGetUniformLocation(program, "modelViewProjection");
    this->matrixShadowLoc = glGetUniformLocation(program, "matrixShadow");
    this->lightDirectionLoc = glGetUniformLocation(program, "lightDirection");
    this->lightEnabledLoc = glGetUniformLocation(program, "lightEnabled");
    this->debugEnabledLoc = glGetUniformLocation(program, "debugEnabled");
    this->triplanarEnabledLoc = glGetUniformLocation(program, "triplanarEnabled");
    this->shadowEnabledLoc = glGetUniformLocation(program, "shadowEnabled");
    this->parallaxEnabledLoc = glGetUniformLocation(program, "parallaxEnabled");
    this->cameraPositionLoc = glGetUniformLocation(program, "cameraPosition");
    this->timeLoc = glGetUniformLocation(program, "time");
    this->shadowMapLoc = glGetUniformLocation(program, "shadowMap");
    this->noiseLoc = glGetUniformLocation(program, "noise");
    this->overrideTextureEnabledLoc = glGetUniformLocation(program, "overrideTextureEnabled");
    this->depthTextureLoc = glGetUniformLocation(program, "depthTexture");
    this->underTextureLoc = glGetUniformLocation(program, "underTexture");
    this->layerLoc = glGetUniformLocation(program, "layer");
}


void ProgramData::uniform(){
    glUniformMatrix4fv(modelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(modelViewProjection));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(matrixShadowLoc, 1, GL_FALSE, glm::value_ptr(matrixShadow  ));
    glUniform3fv(lightDirectionLoc, 1, glm::value_ptr(lightDirection));
    glUniform3fv(cameraPositionLoc, 1, glm::value_ptr(cameraPosition));
    glUniform1f(timeLoc, time);
    glUniform1ui(lightEnabledLoc, lightEnabled);
    glUniform1ui(triplanarEnabledLoc, 1);
    glUniform1ui(parallaxEnabledLoc, parallaxEnabled);
    glUniform1ui(debugEnabledLoc,debugEnabled);
    glUniform1ui(shadowEnabledLoc, shadowEnabled);
}

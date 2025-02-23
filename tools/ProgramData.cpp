#include "tools.hpp"

ProgramData::ProgramData(GLuint program) {
    this->program = program;

    this->shadowMapLoc = glGetUniformLocation(program, "shadowMap");
    this->noiseLoc = glGetUniformLocation(program, "noise");
    this->overrideTextureEnabledLoc = glGetUniformLocation(program, "overrideTextureEnabled");
    this->depthTextureLoc = glGetUniformLocation(program, "depthTexture");
    this->underTextureLoc = glGetUniformLocation(program, "underTexture");

    glGenBuffers(1, &this->ubo);
    glUseProgram(program);
    glUniformBlockBinding(program, glGetUniformBlockIndex(program, "UniformBlock"), 0);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBlock), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void ProgramData::uniform(UniformBlock * block){
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);

    UniformBlock readbackBlock;
    glGetBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UniformBlock), &readbackBlock);
    std::cout << "uniform(" << std::to_string(readbackBlock.layer) << ")" << std::endl;

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UniformBlock), block);  // Update the buffer
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

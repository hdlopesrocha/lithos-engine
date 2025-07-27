#include "gl.hpp"

OutputSSBO::OutputSSBO() {

}

OutputSSBO::~OutputSSBO() {
    //std::cout << "OutputSSBO::~OutputSSBO()" << std::endl;
    if (outputSSBO) {
        glDeleteBuffers(1, &outputSSBO);
    }
}

void OutputSSBO::allocate() {
    //std::cout << "OutputSSBO::allocate()" << std::endl;
    glGenBuffers(1, &outputSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, outputSSBO); 
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ComputeShaderOutput), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind the buffer
}

ComputeShaderOutput OutputSSBO::read() {
    //std::cout << "OutputSSBO::read()" << std::endl;
    ComputeShaderOutput result;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputSSBO);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ComputeShaderOutput), &result);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind the buffer
    return result;
}

void OutputSSBO::reset() {
    //std::cout << "OutputSSBO::reset()" << std::endl;
    ComputeShaderOutput result;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ComputeShaderOutput), &result);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind the buffer
}

#include "gl.hpp"


InputSSBO::InputSSBO() {

}

InputSSBO::~InputSSBO() {
    //std::cout << "InputSSBO::~InputSSBO()" << std::endl;
    if (inputSSBO) {
        glDeleteBuffers(1, &inputSSBO);
    }
}

void InputSSBO::allocate() {
    //std::cout << "InputSSBO::allocate()" << std::endl;
    glGenBuffers(1, &inputSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, inputSSBO); 
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ComputeShaderInput), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind the buffer
}

void InputSSBO::copy(const ComputeShaderInput &input) {
    //std::cout << "InputSSBO::copy()" << std::endl;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ComputeShaderInput), &input);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind the buffer


}
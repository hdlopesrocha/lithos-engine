#include "tools.hpp"


InputSSBO::InputSSBO() {

}

void InputSSBO::allocate() {
    std::cout << "InputSSBO::allocateSSBO()" << std::endl;
    glGenBuffers(1, &inputSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, inputSSBO); 
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ComputeShaderInput), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind the buffer
    std::cout << "InputSSBO::allocateSSBO: Ok!" << std::endl;
}

void InputSSBO::copy(ComputeShaderInput &input) {
    std::cout << "InputSSBO::copy()" << std::endl;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ComputeShaderInput), &input);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind the buffer


}
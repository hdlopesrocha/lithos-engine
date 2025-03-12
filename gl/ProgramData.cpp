#include "gl.hpp"
#include <format>
#include <iostream>
#include <sstream>


ProgramData::ProgramData() {
    ubo= 0;
    glGenBuffers(1, &ubo);  // Generate the buffer
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);  // Bind it as a UBO
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBlock), NULL, GL_DYNAMIC_DRAW);  // Allocate memory
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);  // Bind it to binding point 0
    glBindBuffer(GL_UNIFORM_BUFFER, 0);  // Unbind
}


#include "tools.hpp"

#define MAX_NODES 1000000


GeometrySSBO::GeometrySSBO() {

}

void GeometrySSBO::allocate() {
    std::cout << "GeometrySSBO::allocate()" << std::endl;
    glGenBuffers(1, &vertexSSBO);
    glGenBuffers(1, &indexSSBO);

    
    int vertexCount = MAX_NODES * 18; // 18 vertices per node (6 faces * 3 vertices per face)
    // Allocate big enough buffers
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vertexCount * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexSSBO); 

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indexSSBO); 
    glBufferData(GL_SHADER_STORAGE_BUFFER, vertexCount * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind the buffer
    std::cout << "GeometrySSBO::allocate: Ok!" << std::endl;
}

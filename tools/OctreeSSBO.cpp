#include "tools.hpp"

#define MAX_NODES 10000

OctreeSSBO::OctreeSSBO() : nodesSSBO(0), nodesCount(0) {
    //std::cout << "OctreeSSBO::OctreeSSBO()" << std::endl;
}
OctreeSSBO::~OctreeSSBO() {
    //std::cout << "OctreeSSBO::~OctreeSSBO()" << std::endl;
    if (nodesSSBO) {
        glDeleteBuffers(1, &nodesSSBO);
    }
}

void OctreeSSBO::allocate() {
    //std::cout << "OctreeSSBO::allocate()" << std::endl;
    glGenBuffers(1, &nodesSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nodesSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, nodesSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind the buffer
}

void OctreeSSBO::copy(std::vector<OctreeNodeCubeSerialized> * nodes) {
    nodesCount = nodes->size();
    if (nodesCount > MAX_NODES) {
        std::cerr << "Error: Too many nodes for allocated buffer!" << std::endl;
        return;
    }
    //std::cout << "OctreeSSBO::copy() : " << nodesCount << std::endl;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nodesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, nodes->size()*sizeof(OctreeNodeCubeSerialized), nodes->data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind the buffer
}
#include "tools.hpp"

#define MAX_VERTS 10000000
#define MAX_NODES 10000000


void OctreeSSBO::allocate() {
    std::cout << "OctreeSSBO::allocateSSBO()" << std::endl;
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
    std::cout << "OctreeSSBO::copy() : " << nodesCount << std::endl;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nodesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, nodes->size()*sizeof(OctreeNodeCubeSerialized), nodes->data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind the buffer
}
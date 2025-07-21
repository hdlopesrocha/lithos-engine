#include "tools.hpp"

#define MAX_VERTS 10000000
#define MAX_NODES 10000000


void OctreeSSBO::allocateCopy(OctreeSerialized * octree, std::vector<OctreeNodeCubeSerialized> * nodes) {
    std::cout << "OctreeSSBO::allocateSSBO: " << std::to_string(nodes->size()) << std::endl;
    if(nodesSSBO == 0) {
        glGenBuffers(1, &nodesSSBO);
    }
    if(octreeSSBO == 0) {
        glGenBuffers(1, &octreeSSBO);
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, octreeSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, octreeSSBO); 
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(OctreeSerialized), octree, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nodesSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, nodesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, nodes->size()*sizeof(OctreeNodeCubeSerialized), nodes->data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind the buffer
    std::cout << "OctreeSSBO::allocateSSBO: Ok!" << std::endl;
}

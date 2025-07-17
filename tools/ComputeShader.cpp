#include "tools.hpp"

#define MAX_VERTS 10000000
#define MAX_NODES 10000000


ComputeShader::ComputeShader(GLuint program) : program(program) {

}

void ComputeShader::allocateSSBO() {
    std::cout << "ComputeShader::allocateSSBO()" << std::endl;
    glGenBuffers(1, &vertexSSBO);
    glGenBuffers(1, &indexSSBO);
    glGenBuffers(1, &counterSSBO);
    glGenBuffers(1, &nodesSSBO);
    glGenBuffers(1, &octreeSSBO);

    // Allocate big enough buffers
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_VERTS * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexSSBO); 

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_VERTS * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indexSSBO); 

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, counterSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, counterSSBO); 

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, octreeSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(OctreeSerialized), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, octreeSSBO); 

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nodesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_NODES*sizeof(OctreeNodeSerialized), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, nodesSSBO); 

}

void ComputeShader::writeSSBO(OctreeSerialized * octree, std::vector<OctreeNodeSerialized> * nodes) {
    std::cout << "ComputeShader::writeSSBO: " << nodes->size() << std::endl;
    GLuint zero[2] = {0u, 0u};
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, counterSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * sizeof(GLuint), zero);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, octreeSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(OctreeSerialized), octree);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nodesSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, nodes->size()*sizeof(OctreeNodeSerialized), &nodes->data()[0]);

}


void ComputeShader::dispatch() {
    std::cout << "ComputeShader::dispatch()" << std::endl;

    glUseProgram(program);

    uint totalElements = 10000000;
    uint workgroupSize = 64;
    uint numWorkgroups = (totalElements + workgroupSize - 1) / workgroupSize;

    glDispatchCompute(numWorkgroups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    std::cout << "Ok!" << std::endl;
   
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, counterSSBO);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    GLuint * counter = (GLuint*) glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    GLuint vertexCount = counter[0];
    GLuint indexCount = counter[1];
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    std::cout << "vertexCount = " << std::to_string(vertexCount) <<std::endl;
    std::cout << "indexCount = " << std::to_string(indexCount) <<std::endl;

}
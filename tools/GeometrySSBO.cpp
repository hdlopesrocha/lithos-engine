#include "tools.hpp"

#define MAX_VERTS 10000000
#define MAX_NODES 10000000


GeometrySSBO::GeometrySSBO() {

}

void GeometrySSBO::allocate() {
    std::cout << "GeometrySSBO::allocateSSBO()" << std::endl;
    glGenBuffers(1, &vertexSSBO);
    glGenBuffers(1, &indexSSBO);
    glGenBuffers(1, &counterSSBO);
    glGenBuffers(1, &inputSSBO);

    int vertexCount = nodesCount * 18; // 18 vertices per node (6 faces * 3 vertices per face)
    // Allocate big enough buffers
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vertexCount * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexSSBO); 

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indexSSBO); 
    glBufferData(GL_SHADER_STORAGE_BUFFER, vertexCount * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);

    ComputeShaderOutput result = ComputeShaderOutput(0, 0, glm::vec4(0.0f));
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, counterSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, counterSSBO); 
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ComputeShaderOutput), &result, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, inputSSBO); 
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ComputeShaderInput), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind the buffer
    std::cout << "GeometrySSBO::allocateSSBO: Ok!" << std::endl;
}

void GeometrySSBO::dispatch(GLuint program) {
    std::cout << "GeometrySSBO::dispatch()" << std::endl;

    glUseProgram(program);

    uint totalElements = nodesCount; // or however you track the number of nodes
    uint workgroupSize = 64;
    uint numWorkgroups = (totalElements + workgroupSize - 1) / workgroupSize;

    std::cout << "\tnumWorkgroups = " << std::to_string(numWorkgroups) << std::endl;
    glDispatchCompute(numWorkgroups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "\tglDispatchCompute Error!" << std::endl;
    }else {
        std::cout << "\tglDispatchCompute Ok!" << std::endl;
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, counterSSBO);
    ComputeShaderOutput result = ComputeShaderOutput(0, 0, glm::vec4(0.0f));
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ComputeShaderOutput), &result);

    std::cout << "\tresult4f.x = " << std::to_string(result.result4f.x) << std::endl;
    std::cout << "\tresult4f.y = " << std::to_string(result.result4f.y) << std::endl;
    std::cout << "\tresult4f.z = " << std::to_string(result.result4f.z) << std::endl;
    std::cout << "\tresult4f.w = " << std::to_string(result.result4f.w) << std::endl;
    std::cout << "\tvertexCount = " << std::to_string(result.vertexCount) <<std::endl;
    std::cout << "\tindexCount = " << std::to_string(result.indexCount) <<std::endl;
    std::cout << "GeometrySSBO::dispatch: Ok!" << std::endl;

}
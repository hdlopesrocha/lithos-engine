#include "tools.hpp"

#define MAX_NODES 1000000


ComputeShader::ComputeShader(GLuint program) {
    this->program = program;
}

void ComputeShader::dispatch(size_t nodesCount) {
    std::cout << "ComputeShader::dispatch()" << std::endl;


    uint totalElements = nodesCount; // or however you track the number of nodes
    uint workgroupSize = 64;
    uint numWorkgroups = (totalElements + workgroupSize - 1) / workgroupSize;

    std::cout << "\tnumWorkgroups = " << std::to_string(numWorkgroups) << std::endl;
    glUseProgram(program);
    glDispatchCompute(numWorkgroups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "\tglDispatchCompute Error!" << std::endl;
    }else {
        std::cout << "\tglDispatchCompute Ok!" << std::endl;
    }
  

}
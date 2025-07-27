#include "gl.hpp"




ComputeShaderOutput::ComputeShaderOutput() : result4f0(glm::vec4(0.0f)), result4f1(glm::vec4(0.0f)), vertexCount(0u), indexCount(0u) {}

ComputeShaderOutput::ComputeShaderOutput(GLuint vertexCount, GLuint indexCount, glm::vec4 result4f0, glm::vec4 result4f1) 
    : result4f0(result4f0), result4f1(result4f1), vertexCount(vertexCount), indexCount(indexCount) {}

void ComputeShaderOutput::reset() {
    vertexCount = 0;
    indexCount = 0;
    result4f0 = glm::vec4(0.0f);
    result4f1 = glm::vec4(0.0f);
}

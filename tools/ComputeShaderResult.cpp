#include "tools.hpp"




ComputeShaderOutput::ComputeShaderOutput() : result4f(glm::vec4(0.0f)), vertexCount(0u), indexCount(0u) {}

ComputeShaderOutput::ComputeShaderOutput(GLuint vertexCount, GLuint indexCount, glm::vec4 result4f) 
    : result4f(result4f), vertexCount(vertexCount), indexCount(indexCount) {}

void ComputeShaderOutput::reset() {
    vertexCount = 0;
    indexCount = 0;
    result4f = glm::vec4(0.0f);
}

#include "tools.hpp"




ComputeShaderResult::ComputeShaderResult() : result4f(glm::vec4(0.0f)), vertexCount(0u), indexCount(0u) {}

ComputeShaderResult::ComputeShaderResult(GLuint vertexCount, GLuint indexCount, glm::vec4 result4f) 
    : result4f(result4f), vertexCount(vertexCount), indexCount(indexCount) {}

void ComputeShaderResult::reset() {
    vertexCount = 0;
    indexCount = 0;
    result4f = glm::vec4(0.0f);
}

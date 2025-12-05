#include "gl.hpp"


void DrawElementsIndirectCommand::draw(uint mode, long * count) {
    glBindVertexArray(vertexArrayObject);
    glDrawElementsInstancedBaseVertexBaseInstance(
        mode,
        indexCount,
        GL_UNSIGNED_INT,
        (void*)(uintptr_t)(firstIndex * sizeof(GLuint)),
        instanceCount,
        baseVertex,
        baseInstance
    );
    if (count) {
        *count += instanceCount;
    }
}
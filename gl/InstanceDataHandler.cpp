#include "gl.hpp"


glm::vec3 InstanceDataHandler::getCenter(InstanceData instance) {
    return glm::vec3(instance.matrix[3]);
};

void InstanceDataHandler::bindInstance(GLuint instanceBuffer, std::vector<InstanceData> * instances) {
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
    glBufferData(GL_ARRAY_BUFFER, instances->size() * sizeof(InstanceData), instances->data(), GL_STATIC_DRAW);

    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*) offsetof(InstanceData, shift));
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1); // Enable instancing

    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*) offsetof(InstanceData, animation));
    glEnableVertexAttribArray(5);
    glVertexAttribDivisor(5, 1); // Enable instancing

    for (int i = 0; i < 4; i++) {
        glVertexAttribPointer(6 + i, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), 
            (void*)(offsetof(InstanceData, matrix) + i * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6 + i);
        glVertexAttribDivisor(6 + i, 1); // Enable instancing
    }

}


#include "tools.hpp"

#define MAX_NODES 20000


GeometrySSBO::GeometrySSBO() : vertexSSBO(0), indexSSBO(0), vertexArrayObject(0), vertexCount(0), indexCount(0), instanceBuffer(0) {
    std::cout << "GeometrySSBO::GeometrySSBO()" << std::endl;

}

void GeometrySSBO::allocate() {
    std::cout << "GeometrySSBO::allocate()" << std::endl;
	glGenVertexArrays(1, &vertexArrayObject);
    glGenBuffers(1, &vertexSSBO);
    glGenBuffers(1, &indexSSBO);
    glGenBuffers(1, &instanceBuffer);

    // Bind VAO
    glBindVertexArray(vertexArrayObject);
    std::cout << "Generated VAO ID: " << vertexArrayObject << std::endl;

    int vertexCount = MAX_NODES * 18; // 18 vertices per node (6 faces * 3 vertices per face)
    // Allocate big enough buffers
    glBindBuffer(GL_ARRAY_BUFFER, vertexSSBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexSSBO); 

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexSSBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexCount * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indexSSBO); 

    // Bind vertex buffer for vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, vertexSSBO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoord));

    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*) offsetof(Vertex, brushIndex));

    // Bind vertex buffer for instance data
    InstanceData data = InstanceData();
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(InstanceData), &data, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, instanceBuffer); 

    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*) offsetof(InstanceData, shift));
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1); 

    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*) offsetof(InstanceData, animation));
    glEnableVertexAttribArray(5);
    glVertexAttribDivisor(5, 1); 

    for (int i = 0; i < 4; i++) {
        glVertexAttribPointer(6 + i, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), 
            (void*)(offsetof(InstanceData, matrix) + i * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6 + i);
        glVertexAttribDivisor(6 + i, 1);
    }

    // Unbinds
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); 
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

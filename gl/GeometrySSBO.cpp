#include "gl.hpp"



GeometrySSBO::GeometrySSBO() : vertexSSBO(0), indexSSBO(0), vertexArrayObject(0), vertexCount(0), indexCount(0), instanceSSBO(0) {
    //std::cout << "GeometrySSBO::GeometrySSBO()" << std::endl;

}

GeometrySSBO::~GeometrySSBO() {
    //std::cout << "GeometrySSBO::~GeometrySSBO()" << std::endl;
    if (vertexArrayObject) {
        glDeleteVertexArrays(1, &vertexArrayObject);
    }
    if (vertexSSBO) {
        glDeleteBuffers(1, &vertexSSBO);
    }
    if (indexSSBO) {
        glDeleteBuffers(1, &indexSSBO);
    }
    if (instanceSSBO) {
        glDeleteBuffers(1, &instanceSSBO);
    }
}

void GeometrySSBO::allocate() {
    //std::cout << "GeometrySSBO::allocate("  << std::to_string(nodesCount) << ")" << std::endl;

    glGenVertexArrays(1, &vertexArrayObject);
    glGenBuffers(1, &vertexSSBO);
    glGenBuffers(1, &indexSSBO);
    glGenBuffers(1, &instanceSSBO);

    // Bind VAO
    glBindVertexArray(vertexArrayObject);
    //std::cout << "Generated VAO ID: " << vertexArrayObject << std::endl;

    reset(20000);

    // Allocate big enough buffers
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexSSBO); 

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indexSSBO); 

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, instanceSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, instanceSSBO); 

    // Bind vertex buffer for vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, vertexSSBO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoord));

    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_INT, sizeof(Vertex), (void*) offsetof(Vertex, brushIndex));

    // Bind instance data
    glBindBuffer(GL_ARRAY_BUFFER, instanceSSBO);


    for (int i = 0; i < 4; i++) {
        glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), 
            (void*)(offsetof(InstanceData, matrix) + i * sizeof(glm::vec4)));
        glEnableVertexAttribArray(4 + i);
        glVertexAttribDivisor(4 + i, 1);
    }

    glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*) offsetof(InstanceData, shift));
    glEnableVertexAttribArray(8);
    glVertexAttribDivisor(8, 1); 

    glVertexAttribPointer(9, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*) offsetof(InstanceData, animation));
    glEnableVertexAttribArray(9);
    glVertexAttribDivisor(9, 1); 


    // Unbinds
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); 
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void GeometrySSBO::reset(size_t maxVerts) {
    //std::cout << "GeometrySSBO::reset("  << std::to_string(nodesCount) << ")" << std::endl;
    InstanceData instanceData = InstanceData();
    int vertexCount = maxVerts; 
    // Allocate big enough buffers
    glBindBuffer(GL_ARRAY_BUFFER, vertexSSBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexSSBO); 

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexSSBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexCount * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indexSSBO); 

    glBindBuffer(GL_ARRAY_BUFFER, instanceSSBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(InstanceData), &instanceData, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, instanceSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, instanceSSBO); 
}
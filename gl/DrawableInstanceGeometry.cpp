#include "gl.hpp"

DrawableInstanceGeometry::DrawableInstanceGeometry(Geometry * t, std::vector<glm::mat4> * instances){

	this->indicesCount = t ? t->indices.size() : 0;
	this->instancesCount = instances ? instances->size() : 0;
	if(instancesCount && indicesCount) {
		// Generate buffers and VAO
		glGenVertexArrays(1, &vertexArrayObject);
		glGenBuffers(1, &vertexBuffer);
		glGenBuffers(1, &indexBuffer);
		glGenBuffers(1, &instanceBuffer);

		// Bind VAO
		glBindVertexArray(vertexArrayObject);

		// Index data
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount * sizeof(uint), t->indices.data(), GL_STATIC_DRAW);

		// Vertex data
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, t->vertices.size() * sizeof(Vertex), t->vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoord));
		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*) offsetof(Vertex, brushIndex));
		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Instance data (matrices for instancing)
		glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
		glBufferData(GL_ARRAY_BUFFER, instancesCount * sizeof(glm::mat4), instances->data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(4);
		for (int i = 0; i < 4; i++) {
			glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
			glEnableVertexAttribArray(4 + i);
			glVertexAttribDivisor(4 + i, 1); // Use for instancing
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Unbind VAO
		glBindVertexArray(0);
	}
}

DrawableInstanceGeometry::~DrawableInstanceGeometry() {
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &instanceBuffer);
	glDeleteBuffers(1, &indexBuffer);
	glDeleteVertexArrays(1, &vertexArrayObject);
 }

void DrawableInstanceGeometry::draw(uint mode) {

	if(vertexArrayObject) {
		glBindVertexArray(this->vertexArrayObject);
		glDrawElementsInstanced(mode, this->indicesCount, GL_UNSIGNED_INT, 0, instancesCount);
	    glBindVertexArray(0);
	}
}

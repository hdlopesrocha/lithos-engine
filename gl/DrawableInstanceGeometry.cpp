#include "gl.hpp"

DrawableInstanceGeometry::DrawableInstanceGeometry(Geometry * t, std::vector<InstanceData> * instances, glm::vec3 center){
	this->center = center;
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


		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoord));

		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*) offsetof(Vertex, brushIndex));

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, tangent));
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Instance data (matrices for instancing)
		glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
		glBufferData(GL_ARRAY_BUFFER, instancesCount * sizeof(InstanceData), instances->data(), GL_STATIC_DRAW);

		for (int i = 0; i < 4; i++) {
			glVertexAttribPointer(5 + i, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(sizeof(glm::vec4) * i));
			glEnableVertexAttribArray(5 + i);
			glVertexAttribDivisor(5 + i, 1); // Use for instancing
		}
		glVertexAttribPointer(9, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*) offsetof(InstanceData, shift));
		glEnableVertexAttribArray(9);
		glVertexAttribDivisor(9, 1); // Use for instancing

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

void DrawableInstanceGeometry::draw(uint mode, float amount) {

	if(vertexArrayObject) {
		if (instancesCount <= 0) {
			std::cerr << "Error: instancesCount is " << instancesCount << std::endl;
			return;
		}

		glBindVertexArray(this->vertexArrayObject);
		glDrawElementsInstanced(mode, this->indicesCount, GL_UNSIGNED_INT, 0, instancesCount*amount);
	    glBindVertexArray(0);
	}
}
void DrawableInstanceGeometry::draw(uint mode) {
	draw(mode, 1.0);
}
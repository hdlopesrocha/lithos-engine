#include "gl.hpp"

DrawableInstanceGeometry::DrawableInstanceGeometry(Geometry * t, std::vector<glm::mat4> * instances){

	this->indices = t->indices.size();

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glGenBuffers(1, &ibo);

	glBindVertexArray(vao);

	// Vertex data
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, t->vertices.size()*sizeof(Vertex), t->vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoord));
	glEnableVertexAttribArray(2);		
	glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*) offsetof(Vertex, brushIndex) );
	glEnableVertexAttribArray(3);	
	
	// Instance data
	glBindBuffer(GL_ARRAY_BUFFER, ibo);
	glBufferData(GL_ARRAY_BUFFER, instances->size()*sizeof(glm::mat4), instances, GL_STATIC_DRAW);

	//glEnableVertexAttribArray(4);	
	//glVertexAttribPointer(4, 16, GL_FLOAT, GL_FALSE,sizeof(glm::mat4), (void*) NULL);
	//glVertexAttribDivisor(4,1);

	for (int i = 0; i < 4; i++) {
		glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
		glEnableVertexAttribArray(4 + i);
		glVertexAttribDivisor(4 + i, 1);
	}


	// Index data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, t->indices.size()*sizeof(uint), t->indices.data(), GL_STATIC_DRAW);
	glBindVertexArray(0);

}

DrawableInstanceGeometry::~DrawableInstanceGeometry() {
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);
 }

void DrawableInstanceGeometry::draw(uint mode, GLuint program, std::vector<glm::mat4> * instances) {

	if(this->indices && instances && instances->size()) {
		//std::cout << "Rendering " << std::to_string(this->instances) << std::endl;
		glBindVertexArray(this->vao);
		glDrawElementsInstanced(mode, this->indices, GL_UNSIGNED_INT, 0, instances->size());
	}
}

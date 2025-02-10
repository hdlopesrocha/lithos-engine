#include "gl.hpp"

DrawableGeometry::DrawableGeometry(Geometry * t){
	indices = t->indices.size();
	dirty = false;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, t->vertices.size()*sizeof(Vertex), t->vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, t->indices.size()*sizeof(uint), t->indices.data(), GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoord));
	glEnableVertexAttribArray(2);		
	glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*) offsetof(Vertex, texIndex) );
	glEnableVertexAttribArray(3);
		
}

void DrawableGeometry::draw(uint mode) {
	if(this->indices) {
		glBindVertexArray(this->vao);
		glDrawElements(mode, this->indices, GL_UNSIGNED_INT, 0);
	}
}

GLuint DrawableGeometry::create2DVAO(float x, float y, float w, float h) {
	float vertices[] = {
		// positions   // tex coords
		x, y,    0.0f, 1.0f,
		x, h,    0.0f, 0.0f,
		w, h,    1.0f, 0.0f,
		w, y,    1.0f, 1.0f

	};
	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Vertex position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Texture coordinates
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	return VAO;
}

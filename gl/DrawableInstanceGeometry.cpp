#include "gl.hpp"

DrawableInstanceGeometry::DrawableInstanceGeometry(Geometry * t, std::vector<InstanceData> * instances){

	glm::vec3 geometryCenter = t->vertices.size() ? t->getCenter() : glm::vec3(0);

	this->center = glm::vec3(0);
	int count = instances->size();
	if(instances->size()) {
		float invCount = 1.0f/float(count);
		for(InstanceData &data : *instances){
			this->center += (glm::vec3(data.matrix[3])+geometryCenter)*invCount;
		}
	}

	
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

		
		// Instance data (matrices for instancing)
		glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
		glBufferData(GL_ARRAY_BUFFER, instancesCount * sizeof(InstanceData), instances->data(), GL_STATIC_DRAW);

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
	std::cout << "delete DrawableInstanceGeometry()" << std::endl; 
 }

void DrawableInstanceGeometry::draw(uint mode, float amount, long * count) {

	if(vertexArrayObject) {
		if (instancesCount <= 0) {
			std::cerr << "Error: instancesCount is " << instancesCount << std::endl;
			return;
		}

		glBindVertexArray(this->vertexArrayObject);
		int c =int(ceil(float(instancesCount)*amount));
		*count += c;
		glDrawElementsInstanced(mode, this->indicesCount, GL_UNSIGNED_INT, 0, c);
	    glBindVertexArray(0);
	}
}
void DrawableInstanceGeometry::draw(uint mode, long * count) {
	draw(mode, 1.0, count);
}
#include "gl.hpp"

template <typename T> DrawableInstanceGeometry<T>::DrawableInstanceGeometry(Geometry * t, std::vector<T> * instances, InstanceHandler<T> * handler){


	this->center = glm::vec3(0);
	int count = instances->size();
	glm::vec3 geometryCenter = t->vertices.size() ? t->getCenter() : glm::vec3(0);
	if(instances->size()) {
		float invCount = 1.0f/float(count);
		for(T &data : *instances){
			this->center += (handler->getCenter(data)+geometryCenter)*invCount;
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
		glVertexAttribIPointer(3, 1, GL_INT, sizeof(Vertex), (void*) offsetof(Vertex, brushIndex));

		
		// Instance data (matrices for instancing)
		handler->bindInstance(instanceBuffer, instances);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Unbind VAO
		glBindVertexArray(0);
	}
}

template <typename T> DrawableInstanceGeometry<T>::~DrawableInstanceGeometry() {
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &instanceBuffer);
	glDeleteBuffers(1, &indexBuffer);
	glDeleteVertexArrays(1, &vertexArrayObject);
	//std::cout << "delete DrawableInstanceGeometry()" << std::endl; 
 }

template <typename T> void DrawableInstanceGeometry<T>::draw(uint mode, float amount, long * count) {

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

template <typename T> void DrawableInstanceGeometry<T>::draw(uint mode, long * count) {
	draw(mode, 1.0, count);
}
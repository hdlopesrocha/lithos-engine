#include "tools.hpp"
#include <random>

// Generate a random float between min and max
float randFloat(float min, float max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());  // Mersenne Twister PRNG
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen);
}

InstanceBuffer::InstanceBuffer(int size) {
    this->positions.reserve(size);

    for(int i = 0; i < size ; ++i) {
        this->positions[i] = glm::vec3(randFloat(-50, 50), 0, randFloat(-50, 50));
    }

    glGenBuffers(1, &this->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);  // Per-instance data

}
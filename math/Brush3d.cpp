#include "math.hpp"

Brush3d::Brush3d() {
    this->position = glm::vec3(0);
    this->radius = 1.0f;
    this->mode = BrushMode::ADD;
    this->index = 0;
    this->enabled =true;
}

void Brush3d::reset(Camera * camera){
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f)*camera->quaternion;
    position = camera->position + cameraDirection * (4.0f + radius*2);
}

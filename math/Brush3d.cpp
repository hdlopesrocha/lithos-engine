#include "math.hpp"

Brush3d::Brush3d() {
    this->position = glm::vec3(0);
    this->scale = glm::vec3(1.0f);
    this->mode = BrushMode::ADD;
    this->mode3d = BrushShape::SPHERE;
    this->index = 0;
   	this->translationSensitivity = 32.0f;
    this->enabled =true;
}

void Brush3d::reset(Camera * camera) {
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f)*camera->quaternion;
    position = camera->position + cameraDirection * (4.0f + scale*2.0f);
}

#include "math.hpp"

Brush3d::Brush3d() {
    this->mode = BrushMode::ADD;
    this->index = 0;
   	this->translationSensitivity = 32.0f;
    this->enabled =true;
    this->detail = 1.0;
}

void Brush3d::reset(Camera * camera) {
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f)*camera->quaternion;
    // TODO: Reset brush position based on camera direction
}

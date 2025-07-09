#include "math.hpp"


Camera::Camera(glm::vec3 position, glm::quat quaternion, float near, float far) {
    this->near = near;
    this->far = far;
    this->position = position;
    this->quaternion = quaternion;
	this->rotationSensitivity = 0.01f;
	this->translationSensitivity = 32.0f;
}

glm::vec3 Camera::getCameraDirection() {
    glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
    return glm::normalize(glm::rotate(quaternion, forward));
}

glm::mat4 Camera::getViewProjection() {
    return projection * view;
}


#include "math.hpp"


Camera::Camera(float near, float far) {
    this->near = near;
    this->far = far;
}

glm::vec3 Camera::getCameraDirection() {
    glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
    return glm::normalize(glm::rotate(quaternion, forward));
}

glm::mat4 Camera::getVP() {
    return projection * view;
}


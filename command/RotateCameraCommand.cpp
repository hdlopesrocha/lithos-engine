#include "command.hpp"


RotateCameraCommand::RotateCameraCommand(Camera &camera) : camera(camera) {

}

void RotateCameraCommand::execute(const glm::vec3 &value) {
    glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f);

    camera.quaternion = glm::angleAxis(camera.rotationSensitivity*value.x, xAxis)*camera.quaternion;
    camera.quaternion = glm::angleAxis(camera.rotationSensitivity*value.y, yAxis)*camera.quaternion;
    camera.quaternion = glm::angleAxis(camera.rotationSensitivity*value.z, zAxis)*camera.quaternion;
}

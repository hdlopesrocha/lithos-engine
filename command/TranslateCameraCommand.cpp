#include "command.hpp"

TranslateCameraCommand::TranslateCameraCommand(Camera &camera) : camera(camera) {

}

void TranslateCameraCommand::execute(const glm::vec3 &value) {
    glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f)*camera.quaternion;
    glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f)*camera.quaternion;
    glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, -1.0f)*camera.quaternion;

    camera.position += camera.translationSensitivity*(value.x*xAxis + value.y*yAxis + value.z * zAxis );
}


#include "command.hpp"

TranslateCameraCommand::TranslateCameraCommand(Camera &camera) : camera(camera) {

}

void TranslateCameraCommand::execute(TimedAttribute<glm::vec3> value) {
    glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f)*camera.quaternion;
    glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f)*camera.quaternion;
    glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, -1.0f)*camera.quaternion;

    camera.position += value.deltaTime*camera.translationSensitivity*(value.value.x*xAxis + value.value.y*yAxis + value.value.z * zAxis );
}


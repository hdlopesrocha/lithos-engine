#include "command.hpp"


RotateCameraCommand::RotateCameraCommand(Camera &camera) : camera(camera) {

}

void RotateCameraCommand::execute(TimedAttribute<glm::vec3> value) {
    glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f);

    camera.quaternion = glm::angleAxis(-camera.rotationSensitivity*value.value.z*value.deltaTime, xAxis)*camera.quaternion;
    camera.quaternion = glm::angleAxis(camera.rotationSensitivity*value.value.x*value.deltaTime, yAxis)*camera.quaternion;
    camera.quaternion = glm::angleAxis(camera.rotationSensitivity*value.value.y*value.deltaTime, zAxis)*camera.quaternion;
}

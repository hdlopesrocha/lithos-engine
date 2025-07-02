#include "command.hpp"


RotateCommand::RotateCommand(Camera &camera, glm::quat &quaternion) : camera(camera), quaternion(quaternion) {

}

void RotateCommand::execute(TimedAttribute<glm::vec3> value) {
    glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f);

    quaternion = glm::angleAxis(-camera.rotationSensitivity*value.value.z*value.deltaTime, xAxis)*quaternion;
    quaternion = glm::angleAxis(camera.rotationSensitivity*value.value.x*value.deltaTime, yAxis)*quaternion;
    quaternion = glm::angleAxis(camera.rotationSensitivity*value.value.y*value.deltaTime, zAxis)*quaternion;
}

#include "handler.hpp"


RotateHandler::RotateHandler(Camera &camera, glm::quat &quaternion) : camera(camera), quaternion(quaternion) {

}

void RotateHandler::handle(Axis3dEvent value) {
    glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f);

    quaternion = glm::angleAxis(-camera.rotationSensitivity*value.axis.z*value.deltaTime, xAxis)*quaternion;
    quaternion = glm::angleAxis(camera.rotationSensitivity*value.axis.x*value.deltaTime, yAxis)*quaternion;
    quaternion = glm::angleAxis(camera.rotationSensitivity*value.axis.y*value.deltaTime, zAxis)*quaternion;
}

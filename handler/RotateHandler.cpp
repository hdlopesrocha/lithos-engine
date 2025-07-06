#include "handler.hpp"


RotateHandler::RotateHandler(Camera * camera, glm::quat * quaternion) : camera(camera), quaternion(quaternion) {

}

void RotateHandler::handle(Axis3dEvent * value) {
    glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f);
    float time = value->duration;

    *quaternion = glm::angleAxis(-camera->rotationSensitivity*value->axis.z*time, xAxis)*(*quaternion);
    *quaternion = glm::angleAxis(camera->rotationSensitivity*value->axis.x*time, yAxis)*(*quaternion);
    *quaternion = glm::angleAxis(camera->rotationSensitivity*value->axis.y*time, zAxis)*(*quaternion);
}

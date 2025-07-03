#include "command.hpp"

TranslateCommand::TranslateCommand(Camera &camera, glm::vec3 &vector) : camera(camera), vector(vector) {

}

void TranslateCommand::execute(Axis3dEvent value) {
    glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f)*camera.quaternion;
    glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f)*camera.quaternion;
    glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, -1.0f)*camera.quaternion;

    vector += value.deltaTime*camera.translationSensitivity*(value.axis.x*xAxis + value.axis.y*yAxis + value.axis.z * zAxis );
}


#include "command.hpp"

TranslateBrushCommand::TranslateBrushCommand(Brush3d &brush3d, Camera &camera) : brush3d(brush3d), camera(camera) {

}

void TranslateBrushCommand::execute(glm::vec3 &value) {
    glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f)*camera.quaternion;
    glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f)*camera.quaternion;
    glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, -1.0f)*camera.quaternion;

    //TODO: fix command
    //brush3d.position += brush3d.translationSensitivity*(value.x*xAxis + value.y*yAxis + value.z * zAxis );
}


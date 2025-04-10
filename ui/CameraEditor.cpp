#include "ui.hpp"


CameraEditor::CameraEditor( Camera * camera) {
    this->camera = camera;
}



void CameraEditor::draw2d(float time){
    ImGui::Begin("Camera Editor", &open, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Position: ");
    ImGui::SliderFloat("m##positionX", &camera->position.x, -10000.0f, 10000.0f);
    ImGui::SliderFloat("m##positionY", &camera->position.y, -10000.0f, 10000.0f);
    ImGui::SliderFloat("m##positionZ", &camera->position.z, -10000.0f, 10000.0f);

    ImGui::Text("Quaternion: ");

    ImGui::SliderFloat("##quaternionX", &camera->quaternion.x, -1, 1, "%.3f", 0.01f); 
    ImGui::SliderFloat("##quaternionY", &camera->quaternion.y, -1, 1, "%.3f", 0.01f); 
    ImGui::SliderFloat("##quaternionZ", &camera->quaternion.z, -1, 1, "%.3f", 0.01f); 
    ImGui::SliderFloat("##quaternionW", &camera->quaternion.w, -1, 1, "%.3f", 0.01f); 
    camera->quaternion = glm::normalize(camera->quaternion);

    ImGui::Text("Rotation speed: ");
    ImGui::InputFloat("rad/s##rotationSensitivity", &camera->rotationSensitivity);

    ImGui::Text("Translation speed: ");
    ImGui::InputFloat("m/s##translationSensitivity", &camera->translationSensitivity);

    ImGui::End();
}





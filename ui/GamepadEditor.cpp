#include "ui.hpp"


GamepadEditor::GamepadEditor(TextureImage gamepadTexture) {
    this->gamepadTexture = gamepadTexture;
}



void GamepadEditor::draw2d(float time){
    ImGui::Begin("Gamepad Editor", &open, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Image((ImTextureID)(intptr_t)gamepadTexture.index, ImVec2(gamepadTexture.width/2, gamepadTexture.height/2));


    ImGui::End();
}





#include "ui.hpp"


TextureViewer::TextureViewer(GLuint previewProgram, TextureLayers * layers) {
    this->layers = layers;
    this->previewer = new TexturePreviewer(previewProgram, 256, 256, {"Color", "Normal", "Bump"}, layers);
    this->selectedTexture = 0;
}


void TextureViewer::draw2d(float time){
    ImGui::Begin("Texture Viewer", &open, ImGuiWindowFlags_AlwaysAutoResize);

    previewer->draw2d(selectedTexture);

    ImGui::Text("Selected texture: %d", selectedTexture);
    ImGui::SameLine();

    if (ImGui::ArrowButton("##arrow_left", ImGuiDir_Left)) {
        selectedTexture = Math::mod(selectedTexture - 1, layers->count);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##arrow_right", ImGuiDir_Right)) {
        selectedTexture = Math::mod(selectedTexture + 1, layers->count);
    }

    ImGui::End();
}






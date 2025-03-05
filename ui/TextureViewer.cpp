#include "ui.hpp"


TextureViewer::TextureViewer(GLuint previewProgram, TextureLayers layers) {
    this->layers = layers;
    this->previewer = new TexturePreviewer(previewProgram, 256, 256, {{"Color", layers.textures[0] }, {"Normal", layers.textures[1]}, {"Bump", layers.textures[2] }});
    this->selectedTexture = 0;
}


void TextureViewer::draw2d(){
    ImGui::Begin("Texture Viewer", &open, ImGuiWindowFlags_AlwaysAutoResize);

    previewer->draw2d(selectedTexture);

    ImGui::Text("Selected texture: %d", selectedTexture);
    ImGui::SameLine();

    if (ImGui::ArrowButton("##arrow_left", ImGuiDir_Left)) {
        selectedTexture = Math::mod(selectedTexture - 1, layers.count);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##arrow_right", ImGuiDir_Right)) {
        selectedTexture = Math::mod(selectedTexture + 1, layers.count);
    }

    ImGui::End();
}

void TextureViewer::draw3d(UniformBlock * block){

}





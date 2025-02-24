#include "ui.hpp"
#include <glm/gtc/type_ptr.hpp>


TextureViewer::TextureViewer(std::vector<Texture*> * textures, GLuint previewProgram) {
    this->textures = textures;
    this->previewer = new TexturePreviewer(previewProgram, 256, 256, {"Color", "Normal", "Bump"});
    this->selectedTexture = 0;
}


void TextureViewer::draw2d(){
    ImGui::Begin("Texture Viewer", &open, ImGuiWindowFlags_AlwaysAutoResize);

    previewer->draw2d(textures->at(selectedTexture)->texture);

    ImGui::Text("Selected texture: %d", selectedTexture);
    ImGui::SameLine();

    if (ImGui::ArrowButton("##arrow_left", ImGuiDir_Left)) {
        selectedTexture = Math::mod(selectedTexture - 1, textures->size());
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##arrow_right", ImGuiDir_Right)) {
        selectedTexture = Math::mod(selectedTexture + 1, textures->size());
    }

    ImGui::End();
}

void TextureViewer::draw3d(UniformBlock * block){

}





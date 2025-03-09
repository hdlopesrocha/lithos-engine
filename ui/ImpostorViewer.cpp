#include "ui.hpp"


ImpostorViewer::ImpostorViewer(ImpostorDrawer* impostorDrawer, GLuint previewProgram, int width, int height, TextureLayers * layers) {
    this->impostorDrawer = impostorDrawer;
    this->previewer = new TexturePreviewer(previewProgram, width, height, {"Color", "Normal", "Opacity" }, layers);
    this->selectedDrawer = 0;
    this->layers = layers;
}


void ImpostorViewer::draw2d(){
    ImGui::Begin("Impostor Viewer", &open, ImGuiWindowFlags_AlwaysAutoResize);
    if(layers->count) {
        selectedDrawer = Math::mod(selectedDrawer, layers->count);
        previewer->draw2d(selectedDrawer);
    }
    
    ImGui::Text("Selected impostor: %d/%d ", selectedDrawer, layers->count);
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedDrawer_left", ImGuiDir_Left)) {
        --selectedDrawer;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedDrawer_right", ImGuiDir_Right)) {
        ++selectedDrawer;
    }

    
    ImGui::End();
}

void ImpostorViewer::draw3d(UniformBlock * block){

}





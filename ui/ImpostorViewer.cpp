#include "ui.hpp"


ImpostorViewer::ImpostorViewer(ImpostorDrawer* impostorDrawer, std::vector<ImpostorParams> *impostors,GLuint previewProgram, int width, int height, TextureLayers * layers) {
    this->impostorDrawer = impostorDrawer;
    this->impostors = impostors;
    this->previewer = new TexturePreviewer(previewProgram, width, height, {"Color", "Normal", "Opacity" }, layers);
    this->selectedDrawer = 0;
    this->layers = layers;
}


void ImpostorViewer::draw2d(float time){
    selectedDrawer = Math::mod(selectedDrawer, layers->count);

    ImpostorParams * params = &(*impostors)[selectedDrawer];
    impostorDrawer->draw(*params, time);


    ImGui::Begin("Impostor Viewer", &open, ImGuiWindowFlags_AlwaysAutoResize);
    if(layers->count) {
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



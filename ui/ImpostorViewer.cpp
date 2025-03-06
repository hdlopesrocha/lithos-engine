#include "ui.hpp"


ImpostorViewer::ImpostorViewer(std::vector<ImpostorDrawer*> * impostorDrawers, GLuint previewProgram, int width, int height, TextureLayers * layers) {
    this->impostorDrawers = impostorDrawers;
    this->previewer = new TexturePreviewer(previewProgram, width, height, {"Color", "Normal", "Opacity" }, layers);
    this->selectedDrawer = 0;
}


void ImpostorViewer::draw2d(){
    ImGui::Begin("Impostor Viewer", &open, ImGuiWindowFlags_AlwaysAutoResize);

    selectedDrawer = Math::mod(selectedDrawer, impostorDrawers->size());

    ImpostorDrawer * drawer = impostorDrawers->at(selectedDrawer);
    previewer->draw2d(0);

    ImGui::Text("Selected impostor: %d/%ld ", selectedDrawer, impostorDrawers->size());
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





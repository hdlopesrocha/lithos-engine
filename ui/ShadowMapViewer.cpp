#include "ui.hpp"


ShadowMapViewer::ShadowMapViewer(std::vector<std::pair<RenderBuffer, int>> * shadowBuffers) {
    this->shadowBuffers = shadowBuffers;
    this->selectedBuffer = 0;
}

void ShadowMapViewer::draw2d(){
    ImGui::Begin("Shadow Map Viewer", &open, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Selected buffer: ");
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedBuffer_arrow_left", ImGuiDir_Left)) {
        --selectedBuffer;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedBuffer_right", ImGuiDir_Right)) {
        ++selectedBuffer;
    }

    selectedBuffer = Math::mod(selectedBuffer, shadowBuffers->size());


	ImGui::Image((ImTextureID)(intptr_t)shadowBuffers->at(selectedBuffer).first.depthTexture, ImVec2(512, 512));
	ImGui::End();
}

void ShadowMapViewer::draw3d(UniformBlock * block){

}
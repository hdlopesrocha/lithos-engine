#include "ui.hpp"


ShadowMapViewer::ShadowMapViewer(std::vector<std::pair<RenderBuffer, int>> * shadowBuffers, int width, int height) {
    this->shadowBuffers = shadowBuffers;
    this->selectedBuffer = 0;
    this->width = width;
    this->height = height;
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


	ImGui::Image((ImTextureID)(intptr_t)shadowBuffers->at(selectedBuffer).first.depthTexture.index, ImVec2(width, height));
	ImGui::End();
}

void ShadowMapViewer::draw3d(UniformBlock block){

}
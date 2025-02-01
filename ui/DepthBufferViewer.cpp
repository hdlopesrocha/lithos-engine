#include "ui.hpp"


DepthBufferViewer::DepthBufferViewer(GLuint depthTexture) {
    this->depthTexture = depthTexture;
}

void DepthBufferViewer::draw2d(){
    ImGui::Begin("Depth Buffer Viewer", &open, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Image((ImTextureID)(intptr_t)depthTexture, ImVec2(512, 512));
	ImGui::End();
}

void DepthBufferViewer::draw3d(){

}
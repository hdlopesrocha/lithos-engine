#include "ui.hpp"


ShadowMapViewer::ShadowMapViewer(GLuint shadowTexture) {
    this->shadowTexture = shadowTexture;
}

void ShadowMapViewer::draw2d(){
    ImGui::Begin("Shadow Map Viewer", &open, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Image((ImTextureID)(intptr_t)shadowTexture, ImVec2(512, 512));
	ImGui::End();
}

void ShadowMapViewer::draw3d(UniformBlock * block){

}
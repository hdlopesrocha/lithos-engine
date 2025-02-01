#include "ui.hpp"


ImageViewer::ImageViewer(GLuint texture, int width, int height) {
    this->texture = texture;
    this->width = width;
    this->height = height;
}

void ImageViewer::draw2d(){
    ImGui::Begin("Image Viewer", &open, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2(width, height));
	ImGui::End();
}

void ImageViewer::draw3d(){

}
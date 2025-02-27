#include "ui.hpp"


SettingsEditor::SettingsEditor(Settings * settings) {
    this->settings = settings;

}

void SettingsEditor::draw2d(){
    ImGui::Begin("Settings", &open, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Checkbox("Billboards", &settings->billboardEnabled);
    ImGui::DragInt("Billboard range", &settings->billboardRange, 1, 0, 1024, "%d");
    ImGui::Checkbox("Debug", &settings->debugEnabled);
    ImGui::Checkbox("Light", &settings->lightEnabled);
    ImGui::Checkbox("Opacity", &settings->opacityEnabled);
    ImGui::Checkbox("Parallax", &settings->parallaxEnabled);
    ImGui::Checkbox("Shadow", &settings->shadowEnabled);
    ImGui::Checkbox("Tesselation", &settings->tesselationEnabled);
    ImGui::Checkbox("WireFrame", &settings->wireFrameEnabled);
	ImGui::End();
}

void SettingsEditor::draw3d(UniformBlock * block){

}
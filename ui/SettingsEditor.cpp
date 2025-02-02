#include "ui.hpp"


SettingsEditor::SettingsEditor(Settings * settings) {
    this->settings = settings;

}

void SettingsEditor::draw2d(){
    ImGui::Begin("Settings", &open, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Checkbox("Parallax", &settings->parallaxEnabled);
    ImGui::Checkbox("Light", &settings->lightEnabled);
    ImGui::Checkbox("Shadow", &settings->shadowEnabled);
    ImGui::Checkbox("Debug", &settings->debugEnabled);


	ImGui::End();
}

void SettingsEditor::draw3d(){

}
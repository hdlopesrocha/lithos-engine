#include "ui.hpp"


SettingsEditor::SettingsEditor(Settings * settings) {
    this->settings = settings;

}

void SettingsEditor::draw2d(){
    unsigned int min_value = 0;
    unsigned int max_range = 1024;
    unsigned int max_debug = 10;
    unsigned int max_override = 32;
    int int_value = 0;


    ImGui::Begin("Settings", &open, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Checkbox("Billboards", &settings->billboardEnabled);

    int_value = static_cast<int>(settings->billboardRange);
    if(ImGui::DragScalar("Billboard range", ImGuiDataType_U32, &int_value, 1.0f, &min_value, &max_range,"%u")) {
        settings->billboardRange = static_cast<unsigned int>(int_value);
    }
    ImGui::Checkbox("Debug", &settings->debugEnabled);



    std::vector<std::string> debugModes;
    debugModes.push_back("TextureColor");
    debugModes.push_back("TextureNormal");
    debugModes.push_back("TextureBump");
    debugModes.push_back("Tangent");
    debugModes.push_back("Bitangent");
    debugModes.push_back("Normal");
    debugModes.push_back("SharpNormal");
    debugModes.push_back("WorldNormal");
    debugModes.push_back("Triplanar");
    debugModes.push_back("Depth");
    debugModes.push_back("BlendingWeights");


    if(settings->debugEnabled) {
        for (int i = 0; i < debugModes.size(); i++) {
            if (ImGui::RadioButton(debugModes[i].c_str(), this->selectedDebugMode == i)) {
                this->selectedDebugMode = i;
            }
        }
    }
    ImGui::DragFloat("BlendSharpness", &settings->blendSharpness, 0.01f, 0.0f, 100.0f, "%.2f");

    
    settings->debugMode = this->selectedDebugMode;

    ImGui::Checkbox("Light", &settings->lightEnabled);
    ImGui::Checkbox("Opacity", &settings->opacityEnabled);
    ImGui::Checkbox("Override", &settings->overrideEnabled);

    int_value = static_cast<int>(settings->overrideTexture);
    if(ImGui::DragScalar("Override Texture", ImGuiDataType_U32, &int_value, 1.0f, &min_value, &max_override,"%u")){
        settings->overrideTexture = static_cast<unsigned int>(int_value);
    }

    ImGui::Checkbox("Parallax", &settings->parallaxEnabled);
    ImGui::Checkbox("Shadow", &settings->shadowEnabled);
    ImGui::Checkbox("Tesselation", &settings->tesselationEnabled);
    ImGui::Checkbox("WireFrame", &settings->wireFrameEnabled);
	ImGui::End();
}

void SettingsEditor::draw3d(UniformBlock * block){

}
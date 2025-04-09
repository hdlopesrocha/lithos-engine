#include "ui.hpp"


SettingsEditor::SettingsEditor(Settings * settings) {
    this->settings = settings;
}   


glm::vec3 getYawPitchRoll(const glm::quat &q) {
   return glm::eulerAngles(q); // Returns (pitch, yaw, roll) in radians
}



void SettingsEditor::draw2d(float time){
    unsigned int min_value = 0;
    unsigned int max_range = 1024;
    unsigned int max_override = 32;
    int int_value = 0;


//glm::vec3 ypr = getYawPitchRoll( camera->quaternion) ;

    ImGui::Begin("Settings", &open, ImGuiWindowFlags_AlwaysAutoResize);
  

    ImGui::Checkbox("Billboards", &settings->billboardEnabled);

    ImGui::Checkbox("Solid", &settings->solidEnabled);
    ImGui::Checkbox("Liquid", &settings->liquidEnabled);

    int_value = static_cast<int>(settings->billboardRange);
    if(ImGui::DragScalar("Billboard range", ImGuiDataType_U32, &int_value, 1.0f, &min_value, &max_range,"%u")) {
        settings->billboardRange = static_cast<unsigned int>(int_value);
    }
    ImGui::Checkbox("Debug", &settings->debugEnabled);



    std::vector<std::string> debugModes;
    debugModes.push_back("TextureColor");
    debugModes.push_back("TextureNormal");
    debugModes.push_back("TextureBump");
    debugModes.push_back("TextureCoords");
    debugModes.push_back("Tangent");
    debugModes.push_back("Bitangent");
    debugModes.push_back("Normal");
    debugModes.push_back("SharpNormal");
    debugModes.push_back("WorldNormal");
    debugModes.push_back("NormalMap");
    debugModes.push_back("Depth");
    debugModes.push_back("BlendingWeights");
    debugModes.push_back("TextureWeights");
    debugModes.push_back("DistanceFactor");    
    debugModes.push_back("Parallax UV shift");
    debugModes.push_back("Cartoon");
    debugModes.push_back("Dont correct normal");
    debugModes.push_back("Brush Index");
    debugModes.push_back("Countour Lines");
    debugModes.push_back("Grid Lines");

    

    if(settings->debugEnabled) {
        if (ImGui::BeginCombo("DebugMode", debugModes[settings->debugMode].c_str())) {
            for (uint i = 0; i < debugModes.size(); i++) {
                bool isSelected = (i == settings->debugMode);
                if (ImGui::Selectable(debugModes[i].c_str(), isSelected)) {
                    settings->debugMode = i;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus(); // Highlight selected item
                }
            }
            ImGui::EndCombo();
        }
    }
    ImGui::DragFloat("BlendSharpness", &settings->blendSharpness, 0.01f, 0.0f, 100.0f, "%.2f");

    

    ImGui::Checkbox("Light", &settings->lightEnabled);
    ImGui::Checkbox("Opacity", &settings->opacityEnabled);
    ImGui::Checkbox("Override", &settings->overrideEnabled);

    int_value = static_cast<int>(settings->overrideBrush);
    if(ImGui::DragScalar("Override Texture", ImGuiDataType_U32, &int_value, 1.0f, &min_value, &max_override,"%u")){
        settings->overrideBrush = static_cast<unsigned int>(int_value);
    }

    ImGui::Checkbox("Parallax", &settings->parallaxEnabled);
    ImGui::DragFloat("Parallax Distance", &settings->parallaxDistance, 0.1f, 0.0f, 2048.0f, "%.1f");
    ImGui::DragFloat("Parallax Power", &settings->parallaxPower, 0.1f, 0.0f, 256.0f, "%.1f");


    ImGui::Checkbox("Shadow", &settings->shadowEnabled);
    ImGui::Checkbox("Tesselation", &settings->tesselationEnabled);
    ImGui::Checkbox("Mesh WireFrame", &settings->wireFrameEnabled);
    ImGui::Checkbox("Octree Wireframe", &settings->octreeWireframe);
	ImGui::End();
}

#include "ui.hpp"


TextureMixerEditor::TextureMixerEditor(std::vector<TextureMixer*> * mixers, std::vector<Texture*> * textures, GLuint previewProgram) {
    this->mixers = mixers;
    this->textures = textures;
    this->previewer = new TexturePreviewer(previewProgram, 512, 512, {"Color", "Normal", "Bump"});
    this->selectedMixer = 0;
}


void TextureMixerEditor::draw2d(){
    ImGui::Begin("Texture Mixer", &open, ImGuiWindowFlags_AlwaysAutoResize);


    TextureMixer * mixer = mixers->at(Math::mod(selectedMixer, mixers->size()));
    mixer->mix();

    previewer->draw2d(mixer->getTexture());

    ImGui::Text("Mixer: ");
    ImGui::SameLine();
    if (ImGui::ArrowButton("##mixer_arrow_left", ImGuiDir_Left)) {
        --selectedMixer;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##mixer_arrow_right", ImGuiDir_Right)) {
        ++selectedMixer;
    }



    ImGui::Text("Base texture: ");
    ImGui::SameLine();

    if (ImGui::ArrowButton("##base_arrow_left", ImGuiDir_Left)) {
        --mixer->baseTextureIndex;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##base_arrow_right", ImGuiDir_Right)) {
        ++mixer->baseTextureIndex;
    }

    ImGui::Text("Overlay texture: ");
    ImGui::SameLine();

    if (ImGui::ArrowButton("##overlay_arrow_left", ImGuiDir_Left)) {
        --mixer->overlayTextureIndex;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##overlay_arrow_right", ImGuiDir_Right)) {
        ++mixer->overlayTextureIndex;
    }

float step = 0.1f;
float stepFast = 1.0f; // Faster step when holding Shift

    ImGui::Text("Perlin scale: ");
    ImGui::InputInt("\%##perlinScale", &mixer->perlinScale);

    ImGui::Text("Perlin time: ");
    ImGui::InputFloat("s##perlinTime", &mixer->perlinTime, step, stepFast, "%.10f");

    ImGui::Text("Perlin iterations: ");
    ImGui::InputInt("###perlinIterations", &mixer->perlinIterations);

    ImGui::Text("Perlin lacunarity: ");
    ImGui::InputInt("###lacunarity", &mixer->perlinLacunarity);


    ImGui::Text("Brightness: ");
    ImGui::InputFloat("##brightness", &mixer->brightness, step, stepFast, "%.2f");

    ImGui::Text("Contrast: ");
    ImGui::InputFloat("##contrast", &mixer->contrast, step, stepFast, "%.2f");



	ImGui::End();
}

void TextureMixerEditor::draw3d(UniformBlock * block){

}
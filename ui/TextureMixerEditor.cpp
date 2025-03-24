#include "ui.hpp"


TextureMixerEditor::TextureMixerEditor(TextureMixer * mixer, std::vector<MixerParams> * mixers, GLuint previewProgram, TextureLayers * layers) {
    this->mixers = mixers;
    this->mixer = mixer;
    this->layers = layers;
    this->previewer = new TexturePreviewer(previewProgram, 512, 512, {"Color", "Normal", "Bump"}, layers);
    this->selectedMixer = 0;
}


void TextureMixerEditor::draw2d(float time){
    ImGui::Begin("Texture Mixer", &open, ImGuiWindowFlags_AlwaysAutoResize);

    selectedMixer = Math::mod(selectedMixer, mixers->size());

    MixerParams * params = &(*mixers)[selectedMixer];
    mixer->mix(*params);
    previewer->draw2d(params->targetTexture);

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
        --params->baseTexture;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##base_arrow_right", ImGuiDir_Right)) {
        ++params->baseTexture;
    }

    ImGui::Text("Overlay texture: ");
    ImGui::SameLine();

    if (ImGui::ArrowButton("##overlay_arrow_left", ImGuiDir_Left)) {
        --params->overlayTexture;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##overlay_arrow_right", ImGuiDir_Right)) {
        ++params->overlayTexture;
    }

    params->baseTexture = Math::mod(params->baseTexture, layers->count);
    params->overlayTexture = Math::mod(params->overlayTexture, layers->count);

float step = 0.1f;
float stepFast = 1.0f; // Faster step when holding Shift

    ImGui::Text("Perlin scale: ");
    ImGui::InputInt("\%##perlinScale", &params->perlinScale);

    ImGui::Text("Perlin time: ");
    ImGui::InputFloat("s##perlinTime", &params->perlinTime, step, stepFast, "%.10f");

    ImGui::Text("Perlin iterations: ");
    ImGui::InputInt("###perlinIterations", &params->perlinIterations);

    ImGui::Text("Perlin lacunarity: ");
    ImGui::InputInt("###lacunarity", &params->perlinLacunarity);

    ImGui::Text("Brightness: ");
    ImGui::InputFloat("##brightness", &params->brightness, step, stepFast, "%.2f");

    ImGui::Text("Contrast: ");
    ImGui::InputFloat("##contrast", &params->contrast, step, stepFast, "%.2f");

	ImGui::End();
}

void TextureMixerEditor::draw3d(UniformBlock block){

}
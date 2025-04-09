#include "ui.hpp"


AnimatedTextureEditor::AnimatedTextureEditor(std::vector<AnimateParams> * animations, GLuint previewProgram, int width, int height, TextureLayers * layers) {
    this->animations = animations;
    this->selectedAnimatedTexture = 0;
    this->previewer = new TexturePreviewer(previewProgram, width, height, {"Color", "Normal", "Bump" }, layers);
}

void AnimatedTextureEditor::draw2d(float time){
    ImGui::Begin("Animated Textures", &open, ImGuiWindowFlags_AlwaysAutoResize);

    selectedAnimatedTexture = Math::mod(selectedAnimatedTexture, animations->size());

    AnimateParams * params = &(*animations)[selectedAnimatedTexture];
    
    previewer->draw2d(params->targetTexture);

    ImGui::Text("Animated Texture: ");
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedAnimatedTexture_arrow_left", ImGuiDir_Left)) {
        --selectedAnimatedTexture;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedAnimatedTexture_arrow_right", ImGuiDir_Right)) {
        ++selectedAnimatedTexture;
    }



    if (ImGui::ColorEdit4("Pick a Color", (float*)&params->color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview)) {
        // Do something with the selected color
    }

float step = 0.1f;
float stepFast = 1.0f; // Faster step when holding Shift

    ImGui::Text("Perlin scale: ");
    ImGui::InputInt("\%##perlinScale", &params->perlinScale);

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


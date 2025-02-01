#include "ui.hpp"


AnimatedTextureEditor::AnimatedTextureEditor(std::vector<AnimatedTexture*> * animatedTextures, std::vector<Texture*> * textures, GLuint previewProgram) {
    this->animatedTextures = animatedTextures;
    this->textures = textures;
    this->selectedAnimatedTexture = 0;
    this->previewer = new TexturePreviewer(previewProgram, 512, 512);
}

void AnimatedTextureEditor::show() {
    open = true;
}

void AnimatedTextureEditor::hide(){
    open = false;
}

bool AnimatedTextureEditor::isOpen(){
    return open;
}

void AnimatedTextureEditor::draw2d(){
    ImGui::Begin("Animated Textures", &open, ImGuiWindowFlags_AlwaysAutoResize);

    //Texture * baseTexture = (*textures)[selectedBaseTexture];
    //Texture * overlayTexture = (*textures)[selectedOverlayTexture];

    AnimatedTexture * animatedTexture = (*animatedTextures)[selectedAnimatedTexture];


    previewer->draw2d(animatedTexture->getTexture());


    ImGui::Text("Animated Texture: ");
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedAnimatedTexture_arrow_left", ImGuiDir_Left)) {
        selectedAnimatedTexture = Math::mod(selectedAnimatedTexture - 1, animatedTextures->size());
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedAnimatedTexture_arrow_right", ImGuiDir_Right)) {
        selectedAnimatedTexture = Math::mod(selectedAnimatedTexture + 1, animatedTextures->size());
    }



    if (ImGui::ColorEdit4("Pick a Color", (float*)&animatedTexture->color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview)) {
        // Do something with the selected color
    }

float step = 0.1f;
float stepFast = 1.0f; // Faster step when holding Shift

    ImGui::Text("Perlin scale: ");
    ImGui::InputInt("\%##perlinScale", &animatedTexture->perlinScale);

    ImGui::Text("Perlin iterations: ");
    ImGui::InputInt("###perlinIterations", &animatedTexture->perlinIterations);

    ImGui::Text("Perlin lacunarity: ");
    ImGui::InputInt("###lacunarity", &animatedTexture->perlinLacunarity);


    ImGui::Text("Brightness: ");
    ImGui::InputFloat("##brightness", &animatedTexture->brightness, step, stepFast, "%.2f");

    ImGui::Text("Contrast: ");
    ImGui::InputFloat("##contrast", &animatedTexture->contrast, step, stepFast, "%.2f");



	ImGui::End();
}

void AnimatedTextureEditor::draw3d(){

}
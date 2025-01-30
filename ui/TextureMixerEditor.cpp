#include "ui.hpp"


TextureMixerEditor::TextureMixerEditor(std::vector<TextureMixer*> * mixers, std::vector<Texture*> * textures, GLuint previewProgram) {
    this->mixers = mixers;
    this->textures = textures;
    this->previewProgram = previewProgram;
    this->previewBuffer = createRenderFrameBuffer(512,512);
    this->previewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->selectedMixer = 0;
    this->selectedLayer = 0;
}

void TextureMixerEditor::show() {
    open = true;
}

void TextureMixerEditor::hide(){
    open = false;
}

bool TextureMixerEditor::isOpen(){
    return open;
}

void TextureMixerEditor::draw2d(){
    ImGui::Begin("Texture Mixer", &open, ImGuiWindowFlags_AlwaysAutoResize);

    //Texture * baseTexture = (*textures)[selectedBaseTexture];
    //Texture * overlayTexture = (*textures)[selectedOverlayTexture];

    TextureMixer * mixer = (*mixers)[selectedMixer];
    mixer->mix();
    //textureMixer->mix(baseTexture, overlayTexture);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, previewBuffer.frameBuffer);
    glViewport(0, 0, previewBuffer.width, previewBuffer.height); 

    glUseProgram(previewProgram);
    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D_ARRAY, mixer->getTexture());
    glUniform1i(glGetUniformLocation(previewProgram, "textureSampler"), 0); // Set the sampler uniform
    glUniform1i(glGetUniformLocation(previewProgram, "textureLayer"), selectedLayer); // Set the sampler uniform
    glBindVertexArray(previewVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


    ImGui::Text("Mixer: %d", selectedMixer);
    if (ImGui::ArrowButton("##mixer_arrow_left", ImGuiDir_Left)) {
        selectedMixer = Math::mod(selectedMixer - 1, mixers->size());
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##mixer_arrow_right", ImGuiDir_Right)) {
        selectedMixer = Math::mod(selectedMixer + 1, mixers->size());
    }


    ImGui::Text("Layer: %d", selectedLayer);
    if (ImGui::ArrowButton("##layer_arrow_left", ImGuiDir_Left)) {
        selectedLayer = Math::mod(selectedLayer - 1, 3);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##layer_arrow_right", ImGuiDir_Right)) {
        selectedLayer = Math::mod(selectedLayer + 1, 3);
    }


    ImGui::Text("Result: ");
	ImGui::Image((ImTextureID)(intptr_t)previewBuffer.texture, ImVec2(512, 512));

    ImGui::Text("Base texture: ");
    ImGui::SameLine();

    if (ImGui::ArrowButton("##base_arrow_left", ImGuiDir_Left)) {
        mixer->baseTextureIndex = Math::mod(mixer->baseTextureIndex - 1, textures->size());
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##base_arrow_right", ImGuiDir_Right)) {
        mixer->baseTextureIndex = Math::mod(mixer->baseTextureIndex + 1, textures->size());
    }

    ImGui::Text("Overlay texture: ");
    ImGui::SameLine();

    if (ImGui::ArrowButton("##overlay_arrow_left", ImGuiDir_Left)) {
        mixer->overlayTextureIndex = Math::mod(mixer->overlayTextureIndex - 1, textures->size());
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##overlay_arrow_right", ImGuiDir_Right)) {
        mixer->overlayTextureIndex = Math::mod(mixer->overlayTextureIndex + 1, textures->size());
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

void TextureMixerEditor::draw3d(){

}
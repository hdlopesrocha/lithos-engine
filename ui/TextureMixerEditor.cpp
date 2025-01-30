#include "ui.hpp"


TextureMixerEditor::TextureMixerEditor(TextureMixer * textureMixer, std::vector<Texture*> * textures, GLuint previewProgram) {
    this->textureMixer = textureMixer;
    this->textures = textures;
    this->previewProgram = previewProgram;
    this->previewBuffer = createRenderFrameBuffer(1024,1024);
    this->previewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->selectedBaseTexture = 2;
    this->selectedOverlayTexture = 3;
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

    Texture * baseTexture = (*textures)[selectedBaseTexture];
    Texture * overlayTexture = (*textures)[selectedOverlayTexture];
    textureMixer->mix(baseTexture, overlayTexture);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, previewBuffer.frameBuffer);
    glViewport(0, 0, previewBuffer.width, previewBuffer.height); 

    glUseProgram(previewProgram);
    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureMixer->getTexture());
    glUniform1i(glGetUniformLocation(previewProgram, "textureSampler"), 0); // Set the sampler uniform
    glUniform1i(glGetUniformLocation(previewProgram, "textureLayer"), selectedLayer); // Set the sampler uniform
    glBindVertexArray(previewVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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
        selectedBaseTexture = Math::mod(selectedBaseTexture - 1, textures->size());
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##base_arrow_right", ImGuiDir_Right)) {
        selectedBaseTexture = Math::mod(selectedBaseTexture + 1, textures->size());
    }

    ImGui::Text("Overlay texture: ");
    ImGui::SameLine();

    if (ImGui::ArrowButton("##overlay_arrow_left", ImGuiDir_Left)) {
        selectedOverlayTexture = Math::mod(selectedOverlayTexture - 1, textures->size());
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##overlay_arrow_right", ImGuiDir_Right)) {
        selectedOverlayTexture = Math::mod(selectedOverlayTexture + 1, textures->size());
    }


float step = 0.1f;
float stepFast = 1.0f; // Faster step when holding Shift

    ImGui::Text("Perlin scale: ");
    ImGui::InputInt("\%##perlinScale", &textureMixer->perlinScale);

    ImGui::Text("Perlin time: ");
    ImGui::InputFloat("s##perlinTime", &textureMixer->perlinTime, step, stepFast, "%.10f");

    ImGui::Text("Perlin iterations: ");
    ImGui::InputInt("###perlinIterations", &textureMixer->perlinIterations);

    ImGui::Text("Perlin lacunarity: ");
    ImGui::InputInt("###lacunarity", &textureMixer->perlinLacunarity);


    ImGui::Text("Brightness: ");
    ImGui::InputFloat("##brightness", &textureMixer->brightness, step, stepFast, "%.2f");

    ImGui::Text("Contrast: ");
    ImGui::InputFloat("##contrast", &textureMixer->contrast, step, stepFast, "%.2f");



	ImGui::End();
}

void TextureMixerEditor::draw3d(){

}
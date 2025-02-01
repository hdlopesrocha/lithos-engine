#include "ui.hpp"


AnimatedTextureEditor::AnimatedTextureEditor(std::vector<AnimatedTexture*> * animatedTextures, std::vector<Texture*> * textures, GLuint previewProgram) {
    this->animatedTextures = animatedTextures;
    this->textures = textures;
    this->previewProgram = previewProgram;
    this->previewBuffer = createRenderFrameBuffer(512,512);
    this->previewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->selectedAnimatedTexture = 0;
    this->selectedLayer = 0;
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


    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, previewBuffer.frameBuffer);
    glViewport(0, 0, previewBuffer.width, previewBuffer.height); 

    glUseProgram(previewProgram);
    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D_ARRAY, animatedTexture->getTexture());
    glUniform1i(glGetUniformLocation(previewProgram, "textureSampler"), 0); // Set the sampler uniform
    glUniform1i(glGetUniformLocation(previewProgram, "textureLayer"), selectedLayer); // Set the sampler uniform
    glBindVertexArray(previewVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


    ImGui::Text("Animated Texture: %d", selectedAnimatedTexture);
    if (ImGui::ArrowButton("##selectedAnimatedTexture_arrow_left", ImGuiDir_Left)) {
        selectedAnimatedTexture = Math::mod(selectedAnimatedTexture - 1, animatedTextures->size());
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedAnimatedTexture_arrow_right", ImGuiDir_Right)) {
        selectedAnimatedTexture = Math::mod(selectedAnimatedTexture + 1, animatedTextures->size());
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
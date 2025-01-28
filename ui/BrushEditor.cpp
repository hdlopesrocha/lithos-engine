#include "ui.hpp"


BrushEditor::BrushEditor(std::vector<Texture*> * t, GLuint previewProgram, RenderBuffer previewBuffer, GLuint previewVao) {
    this->textures = t;
    this->previewProgram = previewProgram;
    this->previewBuffer = previewBuffer;
    this->previewVao = previewVao;
}

void BrushEditor::show() {
    open = true;
}

void BrushEditor::hide(){
    open = false;
}
bool BrushEditor::isOpen(){
    return open;
}

int BrushEditor::getSelectedTexture() {
    return selectedTexture;
}


void BrushEditor::render(){
    ImGui::Begin("Brush Editor", &open, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Selected texture: ");
    ImGui::SameLine();

    if (ImGui::ArrowButton("##arrow_left", ImGuiDir_Left)) {
        selectedTexture = Math::mod(selectedTexture - 1, textures->size());
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##arrow_right", ImGuiDir_Right)) {
        selectedTexture = Math::mod(selectedTexture + 1, textures->size());
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, previewBuffer.frameBuffer);
    glUseProgram(previewProgram);
    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D_ARRAY, (*textures)[selectedTexture]->texture);
    glUniform1i(glGetUniformLocation(previewProgram, "textureSampler"), 0); // Set the sampler uniform
    glUniform1i(glGetUniformLocation(previewProgram, "textureLayer"), 0); // Set the sampler uniform
    glBindVertexArray(previewVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    ImGui::Image((ImTextureID)(intptr_t)previewBuffer.texture, ImVec2(200, 200));



    ImGui::End();
}
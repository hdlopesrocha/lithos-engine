#include "ui.hpp"
#include <glm/gtc/type_ptr.hpp>


BrushEditor::BrushEditor(Camera * camera, std::vector<Texture*> * t, GLuint program3d, GLuint previewProgram, RenderBuffer previewBuffer, GLuint previewVao) {
    this->camera = camera;
    this->textures = t;
    this->previewProgram = previewProgram;
    this->previewBuffer = previewBuffer;
    this->previewVao = previewVao;
    SphereGeometry sphereGeometry(20,40);
	this->sphere = new DrawableGeometry(&sphereGeometry);

    this->modelLoc = glGetUniformLocation(program3d, "model");
    this->modelViewProjectionLoc = glGetUniformLocation(program3d, "modelViewProjection");
    this->shadowEnabledLoc = glGetUniformLocation(program3d, "shadowEnabled");
    this->overrideTextureLoc = glGetUniformLocation(program3d, "overrideTexture");
    this->overrideTextureEnabledLoc = glGetUniformLocation(program3d, "overrideTextureEnabled");

    this->brushPosition = glm::vec3(0);
    this->brushRadius = 1.0f;

    this->mode = BrushMode::ADD;
}



void BrushEditor::show() {
    open = true;
    resetPosition();
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

void BrushEditor::resetPosition(){
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f)*camera->quaternion;
    brushPosition = camera->position + cameraDirection * 10.0f;
}


const char* toString(BrushMode v)
{
    switch (v)
    {
        case ADD:     return "Add";
        case REMOVE:  return "Remove";
        case REPLACE: return "Replace";
        default:      return "Unknown";
    }
}

void BrushEditor::draw2d(){
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

    const char* buttonText = "Reset Position";
    ImVec2 textSize = ImGui::CalcTextSize(buttonText);
    float paddingX = ImGui::GetStyle().FramePadding.x;
    float paddingY = ImGui::GetStyle().FramePadding.y;
    ImVec2 buttonSize(textSize.x + paddingX * 2.0f, textSize.y + paddingY * 2.0f);

    if (ImGui::Button(buttonText, buttonSize)) {
        resetPosition();
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

    ImGui::Text("Brush mode: ");

    for (int i = 0; i < BrushMode::COUNT; ++i) {
        BrushMode bm = BrushMode(i);
        std::string label = std::string(toString(bm));
        if(ImGui::RadioButton(label.c_str(), this->mode == bm)){
            this->mode = bm;
        }
    }

    ImGui::End();
}

void BrushEditor::draw3d(){
    glm::mat4 model2 = glm::scale(glm::translate(  glm::mat4(1.0f), brushPosition), glm::vec3(brushRadius));
    glm::mat4 mvp2 = camera->getMVP(model2);
    glUniformMatrix4fv(modelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(mvp2));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
    glUniform1ui(overrideTextureEnabledLoc, 1);
    glUniform1ui(shadowEnabledLoc, 0);
    glUniform1ui(overrideTextureLoc, getSelectedTexture());
    sphere->draw(GL_PATCHES);
    glUniform1ui(overrideTextureEnabledLoc, 0);
}





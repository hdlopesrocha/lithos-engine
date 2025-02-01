#include "ui.hpp"
#include <glm/gtc/type_ptr.hpp>


BrushEditor::BrushEditor(Camera * camera, std::vector<Brush*> * brushes, GLuint program3d, GLuint previewProgram) {
    this->program3d = program3d;
    this->camera = camera;
    this->brushes = brushes;
    this->previewer = new TexturePreviewer(previewProgram, 256, 256);
    SphereGeometry sphereGeometry(20,40);
	this->sphere = new DrawableGeometry(&sphereGeometry);

    this->modelLoc = glGetUniformLocation(program3d, "model");
    this->modelViewProjectionLoc = glGetUniformLocation(program3d, "modelViewProjection");
    this->shadowEnabledLoc = glGetUniformLocation(program3d, "shadowEnabled");
    this->overrideTextureLoc = glGetUniformLocation(program3d, "overrideTexture");
    this->overrideTextureEnabledLoc = glGetUniformLocation(program3d, "overrideTextureEnabled");

    this->brushPosition = glm::vec3(0);
    this->brushRadius = 2.0f;

    this->mode = BrushMode::ADD;
    this->selectedBrush = 6;
    this->brush = (*brushes)[this->selectedBrush];
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

int BrushEditor::getSelectedBrush() {
    return selectedBrush;
}

void BrushEditor::resetPosition(){
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f)*camera->quaternion;
    brushPosition = camera->position + cameraDirection * (4.0f + brushRadius*2);
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

    previewer->draw2d((*brushes)[selectedBrush]->texture->texture);

    ImGui::Text("Selected texture: ");
    ImGui::SameLine();

    if (ImGui::ArrowButton("##arrow_left", ImGuiDir_Left)) {
        selectedBrush = Math::mod(selectedBrush - 1, brushes->size());
        this->brush = (*brushes)[this->selectedBrush];
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##arrow_right", ImGuiDir_Right)) {
        selectedBrush = Math::mod(selectedBrush + 1, brushes->size());
        this->brush = (*brushes)[this->selectedBrush];
    }



    const char* buttonText = "Reset Position";
    ImVec2 textSize = ImGui::CalcTextSize(buttonText);
    float paddingX = ImGui::GetStyle().FramePadding.x;
    float paddingY = ImGui::GetStyle().FramePadding.y;
    ImVec2 buttonSize(textSize.x + paddingX * 2.0f, textSize.y + paddingY * 2.0f);

    if (ImGui::Button(buttonText, buttonSize)) {
        resetPosition();
    }

    ImGui::Text("Mode: ");

    for (int i = 0; i < BrushMode::COUNT; ++i) {
        BrushMode bm = BrushMode(i);
        std::string label = std::string(toString(bm));
        if(ImGui::RadioButton(label.c_str(), this->mode == bm)){
            this->mode = bm;
        }
    }

    ImGui::Text("Position: ");
    ImGui::InputFloat3("m##brushPosition", &brushPosition[0]);
    
    ImGui::Text("Radius: ");
    ImGui::InputFloat("m##brushRadius", &brushRadius);

    ImGui::Text("Texture Scale: ");
    ImGui::InputFloat2("\%##textureScale", &brush->textureScale[0]);

    ImGui::Text("Parallax Scale: ");
    ImGui::InputFloat("m##parallaxScale", &brush->parallaxScale);

    ImGui::Text("Parallax Min Layers: ");
    ImGui::InputFloat("# ##parallaxMinLayers", &brush->parallaxMinLayers);

    ImGui::Text("Parallax Max Layers: ");
    ImGui::InputFloat("# ##parallaxMaxLayers", &brush->parallaxMaxLayers);

    ImGui::Text("Specular Strength: ");
    ImGui::InputFloat("\%##specularStrength", &brush->specularStrength);

    ImGui::Text("Shininess: ");
    ImGui::InputFloat("\%##shininess", &brush->shininess);

    ImGui::End();
}
void BrushEditor::draw3d(){
    glm::mat4 model2 = glm::scale(
        glm::translate(  
            glm::mat4(1.0f), 
            brushPosition
        ), 
        glm::vec3(brushRadius)
    );
    glm::mat4 mvp2 = camera->getMVP(model2);
    glUniformMatrix4fv(modelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(mvp2));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
    glUniform1ui(shadowEnabledLoc, 0);
    glUniform1ui(overrideTextureLoc, getSelectedBrush());
    glUniform1ui(overrideTextureEnabledLoc, 1);
    Brush::bindBrush(program3d, "overrideProps" , brush);
    Brush::bindBrush(program3d, "brushes[" + std::to_string(selectedBrush) + "]" , brush);

    sphere->draw(GL_PATCHES);
    glUniform1ui(overrideTextureEnabledLoc, 0);
}





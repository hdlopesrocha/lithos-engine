#include "ui.hpp"


BrushEditor::BrushEditor(Camera * camera, std::vector<Brush*> * brushes, GLuint program, GLuint previewProgram, TextureLayers layers) {
    this->program = program;
    glUseProgram(program);
    this->data = new ProgramData();
    this->camera = camera;
    this->layers = layers;
    this->brushes = brushes;
    this->previewer = new TexturePreviewer(previewProgram, 256, 256, {
        {"Color", layers.colorTextures }, 
        {"Normal", layers.normalTextures},
        {"Bump", layers.bumpTextures }
    });
    SphereGeometry sphereGeometry(40,80);
	this->sphere = new DrawableGeometry(&sphereGeometry);


    this->brushPosition = glm::vec3(0);
    this->brushRadius = 2.0f;

    this->mode = BrushMode::ADD;
    this->selectedBrush = 0;
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

    selectedBrush = Math::mod(selectedBrush, brushes->size());
    Brush * brush = brushes->at(selectedBrush);

    previewer->draw2d(selectedBrush);

    ImGui::Text("Selected brush: %d", selectedBrush);
    ImGui::SameLine();

    if (ImGui::ArrowButton("##arrow_left", ImGuiDir_Left)) {
        --selectedBrush;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##arrow_right", ImGuiDir_Right)) {
        ++selectedBrush;
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

    ImGui::Text("Parallax Fade: ");
    ImGui::InputFloat("# ##parallaxFade", &brush->parallaxFade);

    ImGui::Text("Parallax Refine: ");
    ImGui::InputFloat("# ##parallaxRefine", &brush->parallaxRefine);

    ImGui::Text("Specular Strength: ");
    ImGui::InputFloat("\%##specularStrength", &brush->specularStrength);

    ImGui::Text("Shininess: ");
    ImGui::InputFloat("\%##shininess", &brush->shininess);

    ImGui::End();
}
void BrushEditor::draw3d(UniformBlock * block){
     selectedBrush = Math::mod(selectedBrush, brushes->size());
    Brush * brush = brushes->at(selectedBrush);

    Brush::bindBrush(program, "brushes[" + std::to_string(selectedBrush) + "]" , "brushTextures["+std::to_string(selectedBrush) + "]", brush);

    glm::mat4 model = glm::scale(
        glm::translate(  
            glm::mat4(1.0f), 
            brushPosition
        ), 
        glm::vec3(brushRadius)
    );

    block->viewProjection = camera->getVP();
    block->world = model;
    block->set(SHADOW_FLAG, false);
    block->set(OVERRIDE_FLAG, true);
    block->uintData.w = (uint) selectedBrush;
    data->uniform(block);
    sphere->draw(GL_PATCHES);
}





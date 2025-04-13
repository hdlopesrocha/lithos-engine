#include "ui.hpp"


BrushEditor::BrushEditor( Brush3d * brush, Camera * camera, std::vector<UniformBlockBrush*> * brushes, GLuint program, GLuint previewProgram, TextureLayers * layers) {
    this->program = program;
    this->camera = camera;
    this->brush = brush;
    this->brushes = brushes;
    this->previewer = new TexturePreviewer(previewProgram, 256, 256, {"Color", "Normal", "Bump" }, layers);
}


int BrushEditor::getSelectedBrush() {
    return brush->index;
}

void BrushEditor::draw2d(float time){
    ImGui::Begin("Brush Editor", &open, ImGuiWindowFlags_AlwaysAutoResize);

    previewer->draw2d(brush->index);

    ImGui::Checkbox("Enabled", &brush->enabled);

    ImGui::Text("Selected texture: %d", brush->index);
    ImGui::SameLine();

    if (ImGui::ArrowButton("##arrow_left", ImGuiDir_Left)) {
        --brush->index;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##arrow_right", ImGuiDir_Right)) {
        ++brush->index;
    }
    brush->index = Math::mod(brush->index, brushes->size());
    UniformBlockBrush * uniformBrush = (*brushes)[brush->index];

    const char* buttonText = "Reset Position";
    ImVec2 textSize = ImGui::CalcTextSize(buttonText);
    float paddingX = ImGui::GetStyle().FramePadding.x;
    float paddingY = ImGui::GetStyle().FramePadding.y;
    ImVec2 buttonSize(textSize.x + paddingX * 2.0f, textSize.y + paddingY * 2.0f);

    if (ImGui::Button(buttonText, buttonSize)) {
        brush->reset(camera);
    }

    ImGui::Text("Mode: ");

    for (int i = 0; i < BrushMode::BrushMode_COUNT; ++i) {
        BrushMode bm = BrushMode(i);
        std::string label = std::string(toString(bm));
        if(ImGui::RadioButton(label.c_str(), brush->mode == bm)){
            brush->mode = bm;
        }
    }

    ImGui::Text("Shape: ");

    for (int i = 0; i < BrushShape::BrushShape_COUNT; ++i) {
        BrushShape bs = BrushShape(i);
        std::string label = std::string(toString(bs));
        if(ImGui::RadioButton(label.c_str(), brush->shape == bs)){
            brush->shape = bs;
        }
    }
    ImGui::Text("Detail: ");
    ImGui::InputFloat("m##brushDetail", &brush->detail);

    ImGui::Text("Position: ");
    ImGui::InputFloat3("m##brushPosition", &brush->position[0]);
    
    ImGui::Text("Scale: ");
    ImGui::InputFloat3("m##brushScale", &brush->scale[0]);

    ImGui::Text("Texture Scale: ");
    ImGui::InputFloat2("\%##textureScale", &uniformBrush->textureScale[0]);

    ImGui::Text("Parallax Scale: ");
    ImGui::InputFloat("m##parallaxScale", &uniformBrush->parallaxScale);

    ImGui::Text("Parallax Min Layers: ");
    ImGui::InputFloat("# ##parallaxMinLayers", &uniformBrush->parallaxMinLayers);

    ImGui::Text("Parallax Max Layers: ");
    ImGui::InputFloat("# ##parallaxMaxLayers", &uniformBrush->parallaxMaxLayers);

    ImGui::Text("Parallax Fade: ");
    ImGui::InputFloat("# ##parallaxFade", &uniformBrush->parallaxFade);

    ImGui::Text("Parallax Refine: ");
    ImGui::InputFloat("# ##parallaxRefine", &uniformBrush->parallaxRefine);

    ImGui::Text("Specular Strength: ");
    ImGui::InputFloat("\%##specularStrength", &uniformBrush->specularStrength);

    ImGui::Text("Shininess: ");
    ImGui::InputFloat("\%##shininess", &uniformBrush->shininess);

    ImGui::End();
}





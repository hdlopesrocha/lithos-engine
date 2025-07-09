#include "ui.hpp"


BrushEditor::BrushEditor( Brush3d * brush, Camera * camera, std::vector<UniformBlockBrush> * brushes, GLuint program, GLuint previewProgram, TextureLayers * layers, Octree * brushSpace, BrushContext * brushContext) {
    this->program = program;
    this->camera = camera;
    this->brush = brush;
    this->brushSpace = brushSpace;
    this->brushes = brushes;
    this->brushContext = brushContext;
    this->previewer = new TexturePreviewer(previewProgram, 256, 256, {"Color", "Normal", "Bump" }, layers);
}


int BrushEditor::getSelectedBrush() {
    return brush->index;
}

void BrushEditor::draw2d(float time){
    bool changed = false;

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
    UniformBlockBrush * uniformBrush = &(*brushes)[brush->index];

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

    if (ImGui::BeginCombo("##selectedFunction", toString(brushContext->currentFunction->getType()))) {
        
        for (SignedDistanceFunction * function : brushContext->functions) {
            SdfType bs = function->getType();
            std::string label = std::string(toString(bs));
       
            if (ImGui::Selectable(label.c_str(), function == brushContext->currentFunction)) {
                brushContext->currentFunction = function;
                changed = true; 
            }
            if (function == brushContext->currentFunction) {
                ImGui::SetItemDefaultFocus(); // Highlight selected item
            }
        }
        

        ImGui::EndCombo();
    }
    
    ImGui::Text("Detail: ");
    if(ImGui::InputFloat("m##brushDetail", &brushContext->detail)) {
        changed = true; 
    }

    switch (brushContext->currentFunction->getType())
    {
        case SdfType::SPHERE:
        {
            SphereDistanceFunction* function = (SphereDistanceFunction*)brushContext->currentFunction;
            ImGui::Text("Sphere Position: ");
            if(ImGui::InputFloat3("m##spherePosition", &(function->center[0]))) {
                changed = true; 
            }
            ImGui::Text("Sphere Radius: ");
            if(ImGui::InputFloat("m##sphereRadius", &(function->radius))) {
                changed = true; 
            }
        }
            break;  
        case SdfType::BOX:
        {
            BoxDistanceFunction* function = (BoxDistanceFunction*)brushContext->currentFunction;
            ImGui::Text("Box Position: "); 
            if(ImGui::InputFloat3("m##boxPosition", &(function->center[0]))) {
                changed = true; 
            }    
            ImGui::Text("Box Length: ");
            if(ImGui::InputFloat3("m##boxLength", &(function->length[0]))) {
                changed = true; 
            }
        }
            break;
        case SdfType::CAPSULE:
        {
            CapsuleDistanceFunction* function = (CapsuleDistanceFunction*)brushContext->currentFunction;

            ImGui::Text("Capsule A: ");
            if(ImGui::InputFloat3("m##capsuleA", &(function->a[0]))) {
                changed = true; 
            }
            ImGui::Text("Capsule B: ");
            if(ImGui::InputFloat3("m##capsuleB", &(function->b[0]))) {
                changed = true; 
            }    
            ImGui::Text("Capsule Radius: ");
            if(ImGui::InputFloat("m##capsuleRadius", &(function->radius))) {
                changed = true; 
            }
            break;
        /* code */
        }
        break;
    
    default:
        break;
    }

    if(changed) {
        brushSpace->root->clear(&brushSpace->allocator, *brushSpace);
        if(brush->mode == BrushMode::ADD) {
       //     brushSpace->add(SphereContainmentHandler(brushContext->boundingVolume), *(brushContext->currentFunction), SimpleBrush(brush->index), brushContext->detail, *(brushContext->simplifier));
        } else if(brush->mode == BrushMode::REMOVE) {
        //    brushSpace->del(SphereContainmentHandler(brushContext->boundingVolume), *(brushContext->currentFunction), SimpleBrush(brush->index), brushContext->detail, *(brushContext->simplifier));
        }
    }

    ImGui::Separator();

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





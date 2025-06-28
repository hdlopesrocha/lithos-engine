#include "ui.hpp"


BrushEditor::BrushEditor( Brush3d * brush, Camera * camera, std::vector<UniformBlockBrush> * brushes, GLuint program, GLuint previewProgram, TextureLayers * layers, Octree * brushSpace) {
    this->program = program;
    this->camera = camera;
    this->brush = brush;
    this->brushSpace = brushSpace;
    this->brushes = brushes;
    this->previewer = new TexturePreviewer(previewProgram, 256, 256, {"Color", "Normal", "Bump" }, layers);
    this->simplifier = new Simplifier(0.99f, 0.01f, true);
    this->boundingVolume = BoundingSphere(glm::vec3(0), 3.0f);

    functions.push_back(new SphereDistanceFunction(glm::vec3(0), 1.0f));
    functions.push_back(new BoxDistanceFunction(glm::vec3(0), glm::vec3(1.0f)));
    functions.push_back(new CapsuleDistanceFunction(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), 1.0f));
    selectedFunction = functions[0];
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

    if (ImGui::BeginCombo("##selectedFunction", toString(selectedFunction->getType()))) {
        
        for (SignedDistanceFunction * function : functions) {
            SdfType bs = function->getType();
            std::string label = std::string(toString(bs));
       
            if (ImGui::Selectable(label.c_str(), function == selectedFunction)) {
                selectedFunction = function;
                changed = true; 
            }
            if (function == selectedFunction) {
                ImGui::SetItemDefaultFocus(); // Highlight selected item
            }
       
       
        }
        

        ImGui::EndCombo();
    }
    
    ImGui::Text("Detail: ");
    if(ImGui::InputFloat("m##brushDetail", &brush->detail)) {
        changed = true; 
    }

    switch (selectedFunction->getType())
    {
        case SdfType::SPHERE:
            ImGui::Text("Sphere Radius: ");
            if(ImGui::InputFloat("m##sphereRadius", &((SphereDistanceFunction*)selectedFunction)->radius)) {
                changed = true; 
            }
            ImGui::Text("Sphere Position: ");
            if(ImGui::InputFloat3("m##spherePosition", &((SphereDistanceFunction*)selectedFunction)->position[0])) {
                changed = true; 
            }
            break;  
        case SdfType::BOX:
            ImGui::Text("Box Length: ");
            if(ImGui::InputFloat3("m##boxLength", &((BoxDistanceFunction*)selectedFunction)->length[0])) {
                changed = true; 
            }
            ImGui::Text("Box Position: "); 
            if(ImGui::InputFloat3("m##boxPosition", &((BoxDistanceFunction*)selectedFunction)->position[0])) {
                changed = true; 
            }
            break;
        case SdfType::CAPSULE:
            ImGui::Text("Capsule Radius: ");
            if(ImGui::InputFloat("m##capsuleRadius", &((CapsuleDistanceFunction*)selectedFunction)->radius)) {
                changed = true; 
            }
            ImGui::Text("Capsule A: ");
            if(ImGui::InputFloat3("m##capsuleA", &((CapsuleDistanceFunction*)selectedFunction)->a[0])) {
                changed = true; 
            }
            ImGui::Text("Capsule B: ");
            if(ImGui::InputFloat3("m##capsuleB", &((CapsuleDistanceFunction*)selectedFunction)->b[0])) {
                changed = true; 
            }
            break;
        /* code */
        break;
    
    default:
        break;
    }

    if(changed) {
        brushSpace->root->clear(&brushSpace->allocator, *brushSpace);
        if(brush->mode == BrushMode::ADD) {
            brushSpace->add(SphereContainmentHandler(boundingVolume), *selectedFunction, SimpleBrush(brush->index), brush->detail, *simplifier);
        } else if(brush->mode == BrushMode::REMOVE) {
            brushSpace->del(SphereContainmentHandler(boundingVolume), *selectedFunction, SimpleBrush(brush->index), brush->detail, *simplifier);
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





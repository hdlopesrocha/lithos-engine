#include "ui.hpp"


BrushEditor::BrushEditor( Brush3d * brush, Camera * camera, std::vector<UniformBlockBrush> * brushes, GLuint program, GLuint previewProgram, TextureLayers * layers, Scene * scene, BrushContext * brushContext) {
    this->program = program;
    this->camera = camera;
    this->brush = brush;
    this->scene = scene;
    this->brushes = brushes;
    this->brushContext = brushContext;
    this->previewer = new TexturePreviewer(previewProgram, 256, 256, {"Color", "Normal", "Bump" }, layers);
}


int BrushEditor::getSelectedBrush() {
    return brushContext->brushIndex;
}

void BrushEditor::draw2d(float time){
    bool changed = false;

    ImGui::Begin("Brush Editor", &open, ImGuiWindowFlags_AlwaysAutoResize);

    previewer->draw2d(brushContext->brushIndex);

    ImGui::Checkbox("Enabled", &brush->enabled);

    ImGui::Text("Selected texture: %d", brushContext->brushIndex);
    ImGui::SameLine();

    if (ImGui::ArrowButton("##arrow_left", ImGuiDir_Left)) {
        --brushContext->brushIndex;
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##arrow_right", ImGuiDir_Right)) {
        ++brushContext->brushIndex;
        changed = true;
    }
    brushContext->brushIndex = Math::mod(brushContext->brushIndex, brushes->size());
    UniformBlockBrush * uniformBrush = &(*brushes)[brushContext->brushIndex];

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
        if(ImGui::RadioButton(label.c_str(), brushContext->mode == bm)){
            brushContext->mode = bm;
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
            ImGui::Text("Position: ");
            if(ImGui::InputFloat3("m##spherePosition", &(function->center[0]))) {
                changed = true; 
            }
            ImGui::Text("Radius: ");
            if(ImGui::InputFloat("m##sphereRadius", &(function->radius))) {
                changed = true; 
            }
        }
            break;  
        case SdfType::BOX:
        {
            BoxDistanceFunction* function = (BoxDistanceFunction*)brushContext->currentFunction;
            ImGui::Text("Position: "); 
            if(ImGui::InputFloat3("m##boxPosition", &(function->center[0]))) {
                changed = true; 
            }    
            ImGui::Text("Length: ");
            if(ImGui::InputFloat3("m##boxLength", &(function->length[0]))) {
                changed = true; 
            }
        }
            break;
        case SdfType::CAPSULE:
        {
            CapsuleDistanceFunction* function = (CapsuleDistanceFunction*)brushContext->currentFunction;

            ImGui::Text("A: ");
            if(ImGui::InputFloat3("m##capsuleA", &(function->a[0]))) {
                changed = true; 
            }
            ImGui::Text("B: ");
            if(ImGui::InputFloat3("m##capsuleB", &(function->b[0]))) {
                changed = true; 
            }    
            ImGui::Text("Radius: ");
            if(ImGui::InputFloat("m##capsuleRadius", &(function->radius))) {
                changed = true; 
            }
            break;
        }
        break;
        case SdfType::OCTAHEDRON:
        {
            OctahedronDistanceFunction* function = (OctahedronDistanceFunction*)brushContext->currentFunction;
            ImGui::Text("Position: ");
            if(ImGui::InputFloat3("m##octahedronPosition", &(function->center[0]))) {
                changed = true; 
            }
            ImGui::Text("Radius: ");
            if(ImGui::InputFloat("m##octahedronRadius", &(function->radius))) {
                changed = true; 
            }
        }
        break;  

        case SdfType::PYRAMID:
        {
            PyramidDistanceFunction* function = (PyramidDistanceFunction*)brushContext->currentFunction;
            ImGui::Text("Base: ");
            if(ImGui::InputFloat3("m##pyramidPosition", &(function->base[0]))) {
                changed = true; 
            }
            ImGui::Text("Height: ");
            if(ImGui::InputFloat("m##pyramidHeight", &(function->height))) {
                changed = true; 
            }
        }
        break;  
    default:
        break;
    }

    if (ImGui::SliderFloat("Pitch", &brushContext->model.pitch, -180.0f, 180.0f)) {
        changed = true; 
    }
    if (ImGui::SliderFloat("Yaw", &brushContext->model.yaw, -180.0f, 180.0f)) {
        changed = true; 
    }
    if (ImGui::SliderFloat("Roll", &brushContext->model.roll, -180.0f, 180.0f)) {
        changed = true; 
    }

    ImGui::Text("Scale: ");
    if(ImGui::InputFloat3("m##scale", &(brushContext->model.scale[0]))) {
        changed = true; 
    }  

    ImGui::Text("Translate: ");
    if(ImGui::InputFloat3("m##translate", &(brushContext->model.translate[0]))) {
        changed = true; 
    }  

    if(changed) {
        Octree * space = scene->brushSpace;
        space->root->clear(&space->allocator, *space);
        scene->brushInfo.clear();
        brushContext->apply(*space, *scene->brushSpaceChangeHandler, true);
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





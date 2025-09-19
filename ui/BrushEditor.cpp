#include "ui.hpp"


BrushEditor::BrushEditor( Brush3d * brush, Camera * camera, std::vector<UniformBlockBrush> * brushes, GLuint program, GLuint previewProgram, TextureLayers * layers, Scene * scene, BrushContext * brushContext, EventManager * eventManager) {
    this->program = program;
    this->camera = camera;
    this->brush = brush;
    this->scene = scene;
    this->brushes = brushes;
    this->brushContext = brushContext;
    this->eventManager = eventManager;    
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

    if (ImGui::BeginCombo("##selectedFunction", brushContext->currentFunction->getLabel())) {
        
        for (WrappedSignedDistanceFunction * function : brushContext->functions) {
            std::string label =  std::string(function->getLabel());
       
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
        case SdfType::CAPSULE:
        {
            CapsuleDistanceFunction* function = (CapsuleDistanceFunction*)brushContext->currentFunction->getFunction();

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
        case SdfType::TORUS:
        {
            TorusDistanceFunction * function = (TorusDistanceFunction*)brushContext->currentFunction->getFunction();
            ImGui::Text("Radius: ");
            if(ImGui::InputFloat2("\%##torusRadius", &(function->radius[0]))) {
                changed = true; 
            }
        }
        break; 
    default:
        break;
    }

    ImGui::Text("Rotate: ");
    glm::vec3 euler = glm::eulerAngles(brushContext->model.quaternion);
    pitch = glm::degrees(euler.x);
    yaw   = glm::degrees(euler.y);
    roll  = glm::degrees(euler.z);

    if (ImGui::SliderFloat("Pitch", &pitch, -180.0f, 180.0f)) {
        changed = true; 
        brushContext->model.quaternion = glm::quat(glm::radians(glm::vec3(pitch, yaw, roll)));
    }
    if (ImGui::SliderFloat("Yaw", &yaw, -90.0f, 90.0f)) {
        changed = true; 
        brushContext->model.quaternion = glm::quat(glm::radians(glm::vec3(pitch, yaw, roll)));
    }
    if (ImGui::SliderFloat("Roll", &roll, -180.0f, 180.0f)) {
        changed = true; 
        brushContext->model.quaternion = glm::quat(glm::radians(glm::vec3(pitch, yaw, roll)));
    }



    ImGui::Text("Scale: ");
    if(ImGui::InputFloat3("m##scale", &(brushContext->model.scale[0]))) {
        changed = true; 
    }  

    ImGui::Text("Translate: ");
    if(ImGui::InputFloat3("m##translate", &(brushContext->model.translate[0]))) {
        changed = true; 
    }  
    ImGui::Separator();

    ImGui::Text("Effect: ");

    if (ImGui::BeginCombo("##selectedEffect", brushContext->currentEffect ? brushContext->currentEffect->getLabel() : "None")) {
        for (WrappedSignedDistanceEffect * effect : brushContext->effects) {
            if (ImGui::Selectable(effect ? effect->getLabel() : "None",  effect == brushContext->currentEffect)) {
                brushContext->currentEffect = effect;
                changed = true; 
            }
            if (effect == brushContext->currentEffect) {
                ImGui::SetItemDefaultFocus(); // Highlight selected item
            }
        }
        ImGui::EndCombo();
    }
    
    if(brushContext->currentEffect) {
        switch (brushContext->currentEffect->getType())
        {
            case SdfType::DISTORT_PERLIN:
            {
                WrappedPerlinDistortDistanceEffect* effect = (WrappedPerlinDistortDistanceEffect*)brushContext->currentEffect;

                ImGui::Text("Amplitude: ");
                if(ImGui::InputFloat("m##distortAmplitude", &(effect->amplitude))) {
                    changed = true; 
                }
                ImGui::Text("Frequency: ");
                if(ImGui::InputFloat("m##distortFrequency", &(effect->frequency))) {
                    changed = true; 
                }
                ImGui::Text("Offset: ");
                if(ImGui::DragFloat3("m##distortOffset", &(effect->offset[0]), 1.0f)) {
                    changed = true; 
                }
                ImGui::Text("Brightness: ");
                if(ImGui::InputFloat("m##brightness", &(effect->brightness))) {
                    changed = true; 
                }
                ImGui::Text("Contrast: ");
                if(ImGui::InputFloat("m##contrast", &(effect->contrast))) {
                    changed = true; 
                }
                break;
            }
            case SdfType::CARVE_PERLIN:
            {
                WrappedPerlinCarveDistanceEffect* effect = (WrappedPerlinCarveDistanceEffect*)brushContext->currentEffect;

                ImGui::Text("Amplitude: ");
                if(ImGui::InputFloat("m##carveAmplitude", &(effect->amplitude))) {
                    changed = true; 
                }
                ImGui::Text("Frequency: ");
                if(ImGui::InputFloat("m##carveFrequency", &(effect->frequency))) {
                    changed = true; 
                }
                ImGui::Text("Offset: ");
                if(ImGui::DragFloat3("m##distortOffset", &(effect->offset[0]), 1.0f)) {
                    changed = true; 
                }    
                ImGui::Text("Threshold: ");
                if(ImGui::InputFloat("m##carveThreshold", &(effect->threshold))) {
                    changed = true; 
                }  
                ImGui::Text("Brightness: ");
                if(ImGui::InputFloat("m##brightness", &(effect->brightness))) {
                    changed = true; 
                }
                ImGui::Text("Contrast: ");
                if(ImGui::InputFloat("m##contrast", &(effect->contrast))) {
                    changed = true; 
                }
                break;
            }
            case SdfType::DISTORT_SINE:
            {
                WrappedSineDistortDistanceEffect* effect = (WrappedSineDistortDistanceEffect*)brushContext->currentEffect;

                ImGui::Text("Amplitude: ");
                if(ImGui::InputFloat("m##carveAmplitude", &(effect->amplitude))) {
                    changed = true; 
                }
                ImGui::Text("Frequency: ");
                if(ImGui::InputFloat("m##carveFrequency", &(effect->frequency))) {
                    changed = true; 
                }
                ImGui::Text("Offset: ");
                if(ImGui::DragFloat3("m##distortOffset", &(effect->offset[0]), 1.0f)) {
                    changed = true; 
                }    
                break;
            }
            case SdfType::CARVE_VORONOI:
            {
                WrappedVoronoiCarveDistanceEffect * effect = (WrappedVoronoiCarveDistanceEffect*)brushContext->currentEffect;

                ImGui::Text("Amplitude: ");
                if(ImGui::InputFloat("m##carveAmplitude", &(effect->amplitude))) {
                    changed = true; 
                }
                ImGui::Text("Cell Size: ");
                if(ImGui::InputFloat("m##cellSize", &(effect->cellSize))) {
                    changed = true; 
                }
                ImGui::Text("Offset: ");
                if(ImGui::DragFloat3("m##distortOffset", &(effect->offset[0]), 1.0f)) {
                    changed = true; 
                }    
                ImGui::Text("Brightness: ");
                if(ImGui::InputFloat("m##brightness", &(effect->brightness))) {
                    changed = true; 
                }
                ImGui::Text("Contrast: ");
                if(ImGui::InputFloat("m##contrast", &(effect->contrast))) {
                    changed = true; 
                }
                break;
            }
        default:
            break;
        }
    }


    ImGui::Separator();

    if(changed) {
        eventManager->publish<Event>(Event(EVENT_BRUSH_CHANGED));
    }

    
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





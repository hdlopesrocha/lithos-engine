#include "ui.hpp"


AtlasPainter::AtlasPainter(std::vector<AtlasParams> * atlasParams, std::vector<AtlasTexture> * atlasTextures, AtlasDrawer * atlasDrawer, GLuint programAtlas, GLuint previewProgram, int width, int height, TextureLayers * layers) {
    this->atlasParams = atlasParams;
    this->atlasDrawer = atlasDrawer;
    this->atlasTextures = atlasTextures;
    this->previewer = new TexturePreviewer(previewProgram, width, height, {"Color", "Normal", "Opacity" }, layers);
    this->selectedDrawer = 0;
    this->selectedDraw = 0;
}

void AtlasPainter::draw2d(float time){
    float PI = glm::pi<float>();

    ImGui::Begin("Atlas Painter", &open, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Selected drawer: %d/%ld ", selectedDrawer, atlasParams->size());
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedDrawer_left", ImGuiDir_Left)) {
        --selectedDrawer;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedDrawer_right", ImGuiDir_Right)) {
        ++selectedDrawer;
    }

    selectedDrawer = Math::mod(selectedDrawer, atlasParams->size());
    AtlasParams * params = &(*atlasParams)[selectedDrawer];

    params->sourceTexture = Math::mod(params->sourceTexture, atlasParams->size());
    AtlasTexture * atlas = params->atlasTexture;

    selectedDraw = Math::mod(selectedDraw, params->draws.size());
    TileDraw * tileDraw = &params->draws[selectedDraw];

    
    atlasDrawer->draw(*params);
    previewer->draw2d(params->targetTexture);

    ImGui::Text("Selected texture: %d/%ld ", params->sourceTexture, atlasParams->size());
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedTexture_left", ImGuiDir_Left)) {
        --params->sourceTexture;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedTexture_right", ImGuiDir_Right)) {
        ++params->sourceTexture;
    }


    ImGui::Text("Selected draw: %d/%ld ", selectedDraw, params->draws.size());
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedDraw_left", ImGuiDir_Left)) {
        --selectedDraw;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedDraw_right", ImGuiDir_Right)) {
        ++selectedDraw;
    }
    ImGui::SameLine();
    if (ImGui::Button("-##popDraws")) { 
        if(params->draws.size()> 1) {
            params->draws.pop_back();
        }
    } 

    ImGui::SameLine();
    if (ImGui::Button("+##pushDraws")) { 
        selectedDraw = params->draws.size();
        params->draws.push_back(TileDraw(0, glm::vec2(1.0), glm::vec2(0.0),glm::vec2(0.0), 0));
    }

    ImGui::Separator();

    ImGui::Text("Selected tile: %d/%ld ", tileDraw->index, atlas->tiles.size());
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedTile_left", ImGuiDir_Left)) {
        --tileDraw->index;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedTile_right", ImGuiDir_Right)) {
        ++tileDraw->index;
    }
    
    ImGui::Text("Pivot: ");
    ImGui::SliderFloat2("##tilePivot", &tileDraw->pivot[0], 0.0 , 1.0, "%0.3f");

    ImGui::Text("Offset: ");
    ImGui::SliderFloat2("##tileOffset", &tileDraw->offset[0], 0.0 , 1.0, "%0.3f");

    ImGui::Text("Size: ");
    ImGui::SliderFloat2("##tileSize", &tileDraw->size[0], 0.0 , 1.0, "%0.3f");

    ImGui::Text("Angle: ");
    ImGui::SliderFloat("##tileAngle", &tileDraw->angle, 0.0 , 2.0*PI, "%0.3f");

    ImGui::End();
}





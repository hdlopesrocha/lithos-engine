#include "ui.hpp"


AtlasPainter::AtlasPainter(std::vector<AtlasTexture*> * atlasTextures, std::vector<AtlasDrawer*> * atlasDrawers, GLuint programAtlas, GLuint previewProgram, int width, int height, TextureLayers layers) {
    this->atlasTextures = atlasTextures;
    this->atlasDrawers = atlasDrawers;
    this->layers = layers;
    this->previewer = new TexturePreviewer(previewProgram, width, height, {{"Color", layers.colorTextures }, {"Normal", layers.normalTextures}, {"Opacity", layers.bumpTextures }});
    this->selectedDrawer = 0;
    this->selectedDraw = 0;
}

void AtlasPainter::draw2d(){
    float PI = glm::pi<float>();

    ImGui::Begin("Atlas Painter", &open, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Selected drawer: %d/%ld ", selectedDrawer, atlasDrawers->size());
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedDrawer_left", ImGuiDir_Left)) {
        --selectedDrawer;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedDrawer_right", ImGuiDir_Right)) {
        ++selectedDrawer;
    }

    selectedDrawer = Math::mod(selectedDrawer, atlasDrawers->size());
    AtlasDrawer * drawer = atlasDrawers->at(selectedDrawer);

    drawer->atlasIndex = Math::mod(drawer->atlasIndex, atlasTextures->size());
    AtlasTexture * atlas = atlasTextures->at(drawer->atlasIndex);

    selectedDraw = Math::mod(selectedDraw, drawer->draws.size());
    TileDraw * tileDraw = &drawer->draws[selectedDraw];

    uint tileIndex = Math::mod(tileDraw->index, atlas->tiles.size());
    Tile * tile = &atlas->tiles[tileIndex];
    
    drawer->draw(drawer->atlasIndex, drawer->draws);
    previewer->draw2d(0);

    ImGui::Text("Selected texture: %d/%ld ", drawer->atlasIndex, atlasTextures->size());
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedTexture_left", ImGuiDir_Left)) {
        --drawer->atlasIndex;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedTexture_right", ImGuiDir_Right)) {
        ++drawer->atlasIndex;
    }


    ImGui::Text("Selected draw: %d/%ld ", selectedDraw, drawer->draws.size());
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
        if(drawer->draws.size()> 1) {
            drawer->draws.pop_back();
        }
    } 

    ImGui::SameLine();
    if (ImGui::Button("+##pushDraws")) { 
        selectedDraw = drawer->draws.size();
        drawer->draws.push_back(TileDraw(0, glm::vec2(1.0), glm::vec2(0.0),glm::vec2(0.0), 0));
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

void AtlasPainter::draw3d(UniformBlock * block){

}





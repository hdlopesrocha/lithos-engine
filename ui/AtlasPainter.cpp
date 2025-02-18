#include "ui.hpp"
#include <glm/gtc/type_ptr.hpp>


AtlasPainter::AtlasPainter(std::vector<AtlasTexture*> * atlasTextures, GLuint programAtlas, GLuint previewProgram, int width, int height) {
    this->atlasTextures = atlasTextures;
    this->drawer = new AtlasDrawer(programAtlas, width, height, atlasTextures);
    this->previewer = new TexturePreviewer(previewProgram, width, height, {"Color", "Normal", "Opacity"});
    this->selectedTexture = 0;

    this->draws.push_back(TileDraw(0, glm::vec2(1.0), glm::vec2(0.0),glm::vec2(0.0), 0));
}


void AtlasPainter::draw2d(){
        float PI = glm::pi<float>();


    ImGui::Begin("Atlas Painter", &open, ImGuiWindowFlags_AlwaysAutoResize);

    selectedTexture = Math::mod(selectedTexture, atlasTextures->size());
    AtlasTexture * atlas = (*atlasTextures)[selectedTexture];

    selectedDraw = Math::mod(selectedDraw, draws.size());
    TileDraw * tileDraw = &draws[selectedDraw];

    uint tileIndex = Math::mod(tileDraw->index, atlas->tiles.size());
    Tile * tile = &atlas->tiles[tileIndex];
    
    drawer->draw(selectedTexture, draws);
    previewer->draw2d(drawer->getTexture());

    ImGui::Text("Selected texture: %d/%ld ", selectedTexture, atlasTextures->size());
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedTexture_left", ImGuiDir_Left)) {
        --selectedTexture;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedTexture_right", ImGuiDir_Right)) {
        ++selectedTexture;
    }


    ImGui::Text("Selected draw: %d/%ld ", selectedDraw, draws.size());
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
        if(draws.size()> 1) {
            draws.pop_back();
        }
    } 

    ImGui::SameLine();
    if (ImGui::Button("+##pushDraws")) { 
        selectedDraw = draws.size();
        draws.push_back(TileDraw(0, glm::vec2(1.0), glm::vec2(0.0),glm::vec2(0.0), 0));
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

void AtlasPainter::draw3d(){

}





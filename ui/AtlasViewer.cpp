#include "ui.hpp"
#include <glm/gtc/type_ptr.hpp>


AtlasViewer::AtlasViewer(std::vector<AtlasTexture*> * textures, AtlasDrawer * drawer, GLuint previewProgram, int width, int height) {
    this->textures = textures;
    this->drawer = drawer;
    this->previewer = new TexturePreviewer(previewProgram, width, height, {"Color", "Normal", "Opacity"});
    this->selectedTexture = 0;
    this->selectedTile = 0;

    this->draws.push_back(TileDraw(0, glm::vec2(1.0), glm::vec2(0.0), 0));
}


void AtlasViewer::draw2d(){
    ImGui::Begin("Atlas Viewer", &open, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Selected texture: ");
    ImGui::SameLine();

    uint atlasIndex =Math::mod(selectedTexture, textures->size());
    AtlasTexture * atlas = (*textures)[atlasIndex];
    uint tileIndex =Math::mod(selectedTile, atlas->tiles.size());
    Tile * tile = &atlas->tiles[tileIndex];
    TileDraw * tileDraw = &this->draws[0];
    tileDraw->offset = tile->offset;
    tileDraw->size = tile->size;
    tileDraw->index = tileIndex;

    drawer->draw(atlas, draws);
    previewer->draw2d(drawer->getTexture());

    ImGui::Text("Selected texture: ");
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedTexture_left", ImGuiDir_Left)) {
        selectedTexture = selectedTexture - 1;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedTexture_right", ImGuiDir_Right)) {
        selectedTexture = selectedTexture + 1;
    }

    ImGui::Text("Selected tile: ");
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedTile_left", ImGuiDir_Left)) {
        selectedTile = selectedTile - 1;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedTile_right", ImGuiDir_Right)) {
        selectedTile = selectedTile + 1;
    }

    ImGui::Text("Offset: ");
    ImGui::InputFloat2("m##tileOffset", &tile->offset[0]);

    ImGui::Text("Size: ");
    ImGui::InputFloat2("m##tileSize", &tile->size[0]);

    ImGui::End();
}

void AtlasViewer::draw3d(){

}





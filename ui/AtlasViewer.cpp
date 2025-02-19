#include "ui.hpp"
#include <glm/gtc/type_ptr.hpp>


AtlasViewer::AtlasViewer(std::vector<AtlasTexture*> * atlasTextures, GLuint programAtlas, GLuint previewProgram, int width, int height) {
    this->atlasTextures = atlasTextures;
    this->drawer = new AtlasDrawer(programAtlas, width, height, atlasTextures);
    this->previewer = new TexturePreviewer(previewProgram, width, height, {"Color", "Normal", "Bump", "Opacity"});
    this->selectedTexture = 0;
    this->selectedTile = 0;
    this->drawer->filterEnabled = false;
    this->draws.push_back(TileDraw(0, glm::vec2(1.0), glm::vec2(0.0), glm::vec2(0), 0));
}


void AtlasViewer::draw2d(){
    ImGui::Begin("Atlas Viewer", &open, ImGuiWindowFlags_AlwaysAutoResize);

    selectedTexture = Math::mod(selectedTexture, atlasTextures->size());
    AtlasTexture * atlas = atlasTextures->at(selectedTexture);
    selectedTile =Math::mod(selectedTile, atlas->tiles.size());

    Tile * tile = &atlas->tiles[selectedTile];
    TileDraw * tileDraw = &this->draws[0];
    tileDraw->offset = tile->offset;
    tileDraw->size = tile->size;
    tileDraw->index = selectedTile;

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

    ImGui::Text("Selected tile: %d/%ld ", selectedTile, atlas->tiles.size());
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedTile_left", ImGuiDir_Left)) {
        --selectedTile;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedTile_right", ImGuiDir_Right)) {
        ++selectedTile;
    }

    ImGui::Text("Offset: ");
    ImGui::InputFloat2("m##tileOffset", &tile->offset[0]);

    ImGui::Text("Size: ");
    ImGui::InputFloat2("m##tileSize", &tile->size[0]);

    ImGui::End();
}

void AtlasViewer::draw3d(){

}





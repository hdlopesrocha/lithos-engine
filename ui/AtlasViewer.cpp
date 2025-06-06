#include "ui.hpp"




AtlasViewer::AtlasViewer(std::vector<AtlasTexture> * atlasTextures, AtlasDrawer * drawer, GLuint programAtlas, GLuint previewProgram, int width, int height, TextureLayers * sourceLayers, GLuint copyProgram) {
    TextureLayers * targetLayers = new TextureLayers();
    targetLayers->textures[0] = createTextureArray(width, height, 1, GL_RGB8);
    targetLayers->textures[1] = createTextureArray(width, height, 1, GL_RGB8);
    targetLayers->textures[2] = createTextureArray(width, height, 1, GL_R8);
    targetLayers->count = 1;

    TextureBlitter * blitter = new TextureBlitter(copyProgram, width, height, {GL_RGB8, GL_R8});
    this->drawer = new AtlasDrawer(programAtlas, width, height, sourceLayers, targetLayers, blitter);
    this->atlasTextures = atlasTextures;
    this->previewer = new TexturePreviewer(previewProgram, width, height, {"Color", "Normal", "Opacity" }, targetLayers);
    this->selectedTexture = 0;
    this->selectedTile = 0;
    this->drawer->filterEnabled = false;
}


void AtlasViewer::draw2d(float time){
    ImGui::Begin("Atlas Viewer", &open, ImGuiWindowFlags_AlwaysAutoResize);

    selectedTexture = Math::mod(selectedTexture, atlasTextures->size());
    AtlasTexture * atlasTexture = &(*atlasTextures)[selectedTexture];

    selectedTile =Math::mod(selectedTile, atlasTexture->tiles.size());

    Tile * tile =  &(atlasTexture->tiles)[selectedTile];

    int targetTexture = 0; //targetLayers has only one layer where the drawer is drawing the preview
    // TODO: change TexturePreviewer in order to draw without extra overhead TextureArray

    AtlasParams params(selectedTexture, targetTexture);
    params.draws.push_back(TileDraw(selectedTile, tile->size, tile->offset, glm::vec2(0), 0));
    
    drawer->draw(params, atlasTextures);
    previewer->draw2d(params.targetTexture);

    ImGui::Text("Selected texture: %d/%ld ", selectedTexture, atlasTextures->size());
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedTexture_left", ImGuiDir_Left)) {
        --selectedTexture;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedTexture_right", ImGuiDir_Right)) {
        ++selectedTexture;
    }

    ImGui::Text("Selected tile: %d/%ld ", selectedTile, atlasTexture->tiles.size());
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



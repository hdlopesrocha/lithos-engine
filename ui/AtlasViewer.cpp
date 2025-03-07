#include "ui.hpp"




AtlasViewer::AtlasViewer(std::vector<AtlasTexture*> * atlasTextures, AtlasDrawer * drawer, GLuint programAtlas, GLuint previewProgram, int width, int height, TextureLayers * sourceLayers , TextureBlitter * blitter ) {
    TextureLayers * targetLayers = new TextureLayers();
    // TODO: dimenstions must have same dimensions of textures, not needed to preview, we could decrease this
    targetLayers->textures[0] = createTextureArray(1024, 1024, 1, GL_RGB8);
    targetLayers->textures[1] = createTextureArray(1024, 1024, 1, GL_RGB8);
    targetLayers->textures[2] = createTextureArray(1024, 1024, 1, GL_RGB8);
    targetLayers->count = 1;

    this->drawer = new AtlasDrawer(programAtlas, 1024, 1024, sourceLayers, targetLayers, blitter);
    this->atlasTextures = atlasTextures;
    this->previewer = new TexturePreviewer(previewProgram, width, height, {"Color", "Normal", "Opacity" }, targetLayers);
    this->selectedTexture = 0;
    this->selectedTile = 0;
    this->drawer->filterEnabled = false;
}


void AtlasViewer::draw2d(){
    ImGui::Begin("Atlas Viewer", &open, ImGuiWindowFlags_AlwaysAutoResize);

    selectedTexture = Math::mod(selectedTexture, atlasTextures->size());
    AtlasTexture * atlasTexture = atlasTextures->at(selectedTexture);

    selectedTile =Math::mod(selectedTile, atlasTexture->tiles.size());

    Tile * tile =  &(atlasTexture->tiles)[selectedTile];

    int targetTexture = 0; //targetLayers has only one layer where the drawer is drawing the preview
    // TODO: change TexturePreviewer in order to draw without extra overhead TextureArray

    AtlasParams params(selectedTexture, targetTexture, atlasTexture);
    params.draws.push_back(TileDraw(selectedTile, tile->size, tile->offset, glm::vec2(0), 0));
    
    drawer->draw(params);
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

    ImGui::Text("Selected tile: %d/%ld ", selectedTile, params.atlasTexture->tiles.size());
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

void AtlasViewer::draw3d(UniformBlock * block){

}





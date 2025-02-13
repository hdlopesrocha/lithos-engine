#include "ui.hpp"
#include <glm/gtc/type_ptr.hpp>


AtlasViewer::AtlasViewer(std::vector<AtlasTexture*> * textures, GLuint previewProgram, int width, int height) {
    this->textures = textures;
    this->previewProgram = previewProgram;
    this->previewBuffer = createRenderFrameBuffer(width,height);
    this->previewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->selectedTexture = 0;
    this->selectedTile = 0;
    this->selectedLayer = 0;
    this->width = width;
    this->height = height;
    this->layers.push_back("Color");
    this->layers.push_back("Normal");
    this->layers.push_back("Opacity");
}


void AtlasViewer::draw2d(){
    ImGui::Begin("Texture Viewer", &open, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Selected texture: ");
    ImGui::SameLine();

    AtlasTexture * atlas = (*textures)[Math::mod(selectedTexture, textures->size())];
    Tile tile = atlas->tiles[Math::mod(selectedTile, atlas->tiles.size())];
    glm::mat3 model = glm::scale(glm::translate( glm::mat4(1.0), glm::vec3(tile.offset, 0.0) ), glm::vec3(tile.size, 1.0));

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, previewBuffer.frameBuffer);
    glViewport(0, 0, previewBuffer.width, previewBuffer.height); 
	glClearColor (0.0,0.0,0.0,0.0);    
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(previewProgram);
    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D_ARRAY, atlas->texture);
    glUniform1i(glGetUniformLocation(previewProgram, "textureSampler"), 0); // Set the sampler uniform
    glUniform1i(glGetUniformLocation(previewProgram, "layerIndex"), selectedLayer); // Set the sampler uniform
   	glUniform2fv(glGetUniformLocation(previewProgram, "tileOffset"), 1, glm::value_ptr(tile.offset));
	glUniform2fv(glGetUniformLocation(previewProgram, "tileSize"), 1, glm::value_ptr(tile.size));
	glUniformMatrix3fv(glGetUniformLocation(previewProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(previewVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


    if (ImGui::BeginTabBar("layerPicker_tab")) {
        for(int i=0 ; i < layers.size(); ++i) {
            std::string name = layers[i];
            if (ImGui::BeginTabItem(name.c_str())) {
                selectedLayer = i;
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
	ImGui::Image((ImTextureID)(intptr_t)previewBuffer.colorTexture, ImVec2(width, height));


    ImGui::Text("Selected texture: ");
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedTexture_left", ImGuiDir_Left)) {
        selectedTexture = selectedTexture - 1;
        selectedTile = 0;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##selectedTexture_right", ImGuiDir_Right)) {
        selectedTexture = selectedTexture + 1;
        selectedTile = 0;
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

    ImGui::End();
}

void AtlasViewer::draw3d(){

}





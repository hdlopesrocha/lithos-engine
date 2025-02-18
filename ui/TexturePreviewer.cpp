#include "ui.hpp"


TexturePreviewer::TexturePreviewer(GLuint previewProgram, int width, int height, std::initializer_list<std::string> layers) {
    this->previewProgram = previewProgram;
    this->previewBuffer = createRenderFrameBufferWithoutDepth(width,height);
    this->previewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->selectedLayer = 0;
    this->width = width;
    this->height = height;
    for(std::string name : layers) {
        this->layers.push_back(name);
    }
}

void TexturePreviewer::draw2d(TextureArray texture){
    glUseProgram(previewProgram);
    glBindFramebuffer(GL_FRAMEBUFFER, previewBuffer.frameBuffer);
    glViewport(0, 0, previewBuffer.width, previewBuffer.height); 
	glClearColor (0.0,0.0,0.0,0.0);    
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glUniform1i(glGetUniformLocation(previewProgram, "textureSampler"), 0); 
    glUniform1i(glGetUniformLocation(previewProgram, "textureLayer"), selectedLayer); 
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
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	ImGui::Image((ImTextureID)(intptr_t)previewBuffer.colorTexture, ImVec2(width, height));
}
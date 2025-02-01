#include "ui.hpp"


TexturePreviewer::TexturePreviewer(GLuint previewProgram, int width, int height) {
    this->previewProgram = previewProgram;
    this->previewBuffer = createRenderFrameBuffer(width,height);
    this->previewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->selectedLayer = 0;
    this->width = width;
    this->height = height;
}

void TexturePreviewer::draw2d(TextureArray texture){
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, previewBuffer.frameBuffer);
    glViewport(0, 0, previewBuffer.width, previewBuffer.height); 
	glClearColor (0.0,0.0,0.0,0.0);    
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(previewProgram);
    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glUniform1i(glGetUniformLocation(previewProgram, "textureSampler"), 0); // Set the sampler uniform
    glUniform1i(glGetUniformLocation(previewProgram, "textureLayer"), selectedLayer); // Set the sampler uniform
    glBindVertexArray(previewVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


    if (ImGui::BeginTabBar("layerPicker_tab")) {
        if (ImGui::BeginTabItem("Color")) {
            selectedLayer = 0;
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Normal")) {
            selectedLayer = 1;
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Bump")) {
            selectedLayer = 2;
            ImGui::EndTabItem();
        }
    
        ImGui::EndTabBar();
    }


	ImGui::Image((ImTextureID)(intptr_t)previewBuffer.colorTexture, ImVec2(width, height));


}
#include "ui.hpp"


DepthBufferViewer::DepthBufferViewer(GLuint previewProgram, TextureImage depthTexture, int width, int height) {
    this->depthTexture = depthTexture;
    this->previewProgram = previewProgram;
    this->previewBuffer = createRenderFrameBuffer(width,height, false);
    this->previewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->width = width;
    this->height = height;
}

void DepthBufferViewer::draw2d(float time){
    glBindFramebuffer(GL_FRAMEBUFFER, previewBuffer.frameBuffer);
    glViewport(0, 0, previewBuffer.width, previewBuffer.height); 
	glClearColor (0.0,0.0,0.0,0.0);    
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(previewProgram);
    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D, depthTexture.index);
    glUniform1i(glGetUniformLocation(previewProgram, "textureSampler"), 0); 
    glBindVertexArray(previewVao);
    glDisable(GL_CULL_FACE);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


    ImGui::Begin("Depth Buffer Viewer", &open, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Image((ImTextureID)(intptr_t)previewBuffer.colorTexture.index, ImVec2(width, height));
	ImGui::End();

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}


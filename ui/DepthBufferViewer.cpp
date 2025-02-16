#include "ui.hpp"


DepthBufferViewer::DepthBufferViewer(GLuint previewProgram, GLuint depthTexture, int width, int height) {
    this->depthTexture = depthTexture;
    this->previewProgram = previewProgram;
    this->previewBuffer = createRenderFrameBuffer(width,height);
    this->previewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->width = width;
    this->height = height;
}

void DepthBufferViewer::draw2d(){

    glBindFramebuffer(GL_FRAMEBUFFER, previewBuffer.frameBuffer);
    glViewport(0, 0, previewBuffer.width, previewBuffer.height); 
	glClearColor (0.0,0.0,0.0,0.0);    
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(previewProgram);
    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUniform1i(glGetUniformLocation(previewProgram, "textureSampler"), 0); // Set the sampler uniform
    glBindVertexArray(previewVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


    ImGui::Begin("Depth Buffer Viewer", &open, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Image((ImTextureID)(intptr_t)previewBuffer.colorTexture, ImVec2(width, height));
	ImGui::End();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void DepthBufferViewer::draw3d(){

}
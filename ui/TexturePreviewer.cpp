#include "ui.hpp"


TexturePreviewer::TexturePreviewer(GLuint previewProgram, int width, int height, std::initializer_list<std::string> layerNames, TextureLayers * layers) {
    this->previewProgram = previewProgram;
    this->previewBuffer = createRenderFrameBufferWithoutDepth(width,height);
    this->previewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->selectedLayer = 0;
    this->width = width;
    this->height = height;
    this->layers = layers;
    for(std::string layerName : layerNames) {
        this->layerNames.push_back(layerName);
    }
}

void TexturePreviewer::draw2d(int index){
    glUseProgram(previewProgram);
    glBindFramebuffer(GL_FRAMEBUFFER, previewBuffer.frameBuffer);
    glViewport(0, 0, previewBuffer.width, previewBuffer.height); 
	glClearColor (0.0,0.0,0.0,0.0);    
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    
    for(size_t i = 0; i < layerNames.size() ; ++i) {
        glActiveTexture(GL_TEXTURE0+ i); 
        glBindTexture(GL_TEXTURE_2D_ARRAY, layers->textures[i].index);
        glUniform1i(glGetUniformLocation(previewProgram, ("textures[" + std::to_string(i) + "]").c_str()), i);
    }

    glUniform1ui(glGetUniformLocation(previewProgram, "layer"), selectedLayer); 
    glUniform1ui(glGetUniformLocation(previewProgram, "mono"), selectedLayer == 2); 
    glUniform1ui(glGetUniformLocation(previewProgram, "index"), index); 
    glBindVertexArray(previewVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    if (ImGui::BeginTabBar("layerPicker_tab")) {
        for(size_t i=0 ; i < layerNames.size(); ++i) {
            std::string name = layerNames[i];
            if (ImGui::BeginTabItem(name.c_str())) {
                selectedLayer = i;
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	ImGui::Image((ImTextureID)(intptr_t)previewBuffer.colorTexture.index, ImVec2(width, height));
}
#include "gl.hpp"


ImpostorParams::ImpostorParams(int targetTexture, DrawableInstanceGeometry * mesh){
    this->targetTexture = targetTexture;
    this->mesh = mesh;
}

ImpostorDrawer::ImpostorDrawer(GLuint program, int width, int height, TextureLayers* sourceLayers, TextureLayers * targetLayers, TextureBlitter * blitter) {
    this->height = height;
    this->width = width;
    this->program = program;
    this->blitter = blitter;
    this->sourceLayers = sourceLayers;
    this->targetLayers = targetLayers;
    this->renderBuffer = createMultiLayerRenderFrameBuffer(width, height, 3, 3, true, GL_RGB8);
}

TextureArray ImpostorDrawer::getTexture() {
    return renderBuffer.colorTexture;
}

void ImpostorDrawer::draw(ImpostorParams &params) {
    std::cout << "ImpostorDrawer::draw" << std::endl;
    
    glm::mat4 view = glm::lookAt(glm::vec3(2.0), params.mesh->center, glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), width / (float) height, 0.1f, 32.0f);
    glm::mat4 viewProjection = projection * view;
    glm::mat4 model(1.0);
    
    glUseProgram(program);
    glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer.frameBuffer);
    glViewport(0, 0, renderBuffer.width, renderBuffer.height);
    glClearColor (1.0,0.0,0.0,1.0);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniform1i(glGetUniformLocation(program, "triplanarEnabled"), false); 
    glUniform1i(glGetUniformLocation(program, "opacityEnabled"), false); // TODO: true
    glUniform1i(glGetUniformLocation(program, "overrideEnabled"), false); 
    glUniform1ui(glGetUniformLocation(program, "overrideTexture"), 0u); 
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(program, "viewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));
   
    glDisable(GL_CULL_FACE);
    params.mesh->draw(GL_TRIANGLES);
    glEnable(GL_CULL_FACE);

    for(int i=0 ; i < 3 ; ++i ) {
        blitter->blit(&renderBuffer, i, &targetLayers->textures[i], params.targetTexture);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
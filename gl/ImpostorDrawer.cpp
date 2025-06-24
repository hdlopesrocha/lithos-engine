#include "gl.hpp"


ImpostorParams::ImpostorParams(int targetTexture, DrawableInstanceGeometry<InstanceData> * mesh){
    this->targetTexture = targetTexture;
    this->mesh = mesh;
}

ImpostorDrawer::ImpostorDrawer(GLuint program, int width, int height, TextureLayers* sourceLayers, TextureLayers * targetLayers, TextureBlitter * blitter) {
    this->program = program;
    this->blitter = blitter;
    this->sourceLayers = sourceLayers;
    this->targetLayers = targetLayers;
    this->renderBuffer = createMultiLayerRenderFrameBuffer(width, height, 3, 3, true, GL_RGB8);
}


void ImpostorDrawer::draw(ImpostorParams &params, float time) {
    std::cout << "ImpostorDrawer::draw t=" << std::to_string(time) << std::endl;
    
    glm::mat4 view = glm::lookAt(glm::vec3(10.0 * glm::sin(time), 10.0, 10.0* glm::cos(time)), glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), renderBuffer.width / (float) renderBuffer.height, 0.1f, 128.0f);
    glm::mat4 viewProjection = projection * view;
    glm::mat4 world(1.0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer.frameBuffer);
    glViewport(0, 0, renderBuffer.width, renderBuffer.height);
    glClearColor (1.0,0.0,0.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program);
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "OpenGL error: " << err << std::endl;
    }
    glUniform1i(glGetUniformLocation(program, "opacityEnabled"), false); // TODO: true
    glUniform1i(glGetUniformLocation(program, "targetLayer"), 0); 
    glUniformMatrix4fv(glGetUniformLocation(program, "world"), 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(program, "viewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));
   
    for (int i = 0; i < 3; i++) {
        //std::cout << "Texture Unit " << std::to_string(sourceLayers->textures[i].index) << std::endl;
    }

    long count = 0;
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    params.mesh->draw(GL_TRIANGLES, &count);
    //std::cout << "Drawing " << count << " instances." << std::endl;

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    for(int i=0 ; i < TEXTURE_TYPE_COUNT ; ++i ) {
       // std::cout << "TextureBlitter::blit " << std::to_string(targetLayers->textures[i].index) << " | " << std::to_string(params.targetTexture) << std::endl;
        blitter->blit(&renderBuffer, i, &targetLayers->textures[i], params.targetTexture);
    }


}
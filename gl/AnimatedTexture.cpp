#include "gl.hpp"


AnimateParams::AnimateParams(int targetTexture) {
    this->targetTexture = targetTexture;
    this->perlinScale = 9;
    this->perlinIterations = 8;
    this->perlinLacunarity = 3;
    this->brightness = 0;
    this->contrast = 1;
    this->color = glm::vec4(64/255.0f, 64/255.0f, 64/255.0f, 1.0);    
}

AnimatedTexture::AnimatedTexture(int width, int height, GLuint program, TextureLayers * layers, TextureBlitter * blitter) {
    this->textureMixerBuffer = createMultiLayerRenderFrameBuffer(width,height, 3, 3,false, GL_RGB8);
    this->program = program;
    this->previewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->blitter = blitter;
    this->layers = layers;
}

TextureArray AnimatedTexture::getTexture(){
    return textureMixerBuffer.colorTexture;
}

void AnimatedTexture::animate(float time, AnimateParams params){

    glUseProgram(program);

    glBindFramebuffer(GL_FRAMEBUFFER, textureMixerBuffer.frameBuffer);
    glViewport(0, 0, textureMixerBuffer.width, textureMixerBuffer.height);
	glClearColor (0.0,0.0,0.0,0.0);    
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUniform1i(glGetUniformLocation(program, "perlinScale"), params.perlinScale); 
    glUniform1f(glGetUniformLocation(program, "perlinTime"), time); 
    glUniform1i(glGetUniformLocation(program, "perlinIterations"), params.perlinIterations); 
    glUniform1i(glGetUniformLocation(program, "perlinLacunarity"), params.perlinLacunarity); 
    glUniform1f(glGetUniformLocation(program, "brightness"), params.brightness); 
    glUniform1f(glGetUniformLocation(program, "contrast"), params.contrast); 
    glUniform4fv(glGetUniformLocation(program, "color"), 1, glm::value_ptr(params.color)); 
    glBindVertexArray(previewVao);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    for(int i = 0 ; i < 3 ; ++i) {
        blitter->blit(&textureMixerBuffer, i, &layers->textures[i], params.targetTexture);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

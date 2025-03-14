#include "gl.hpp"

TextureMixer::TextureMixer(int width, int height, GLuint program, TextureLayers * layers, TextureBlitter * blitter) {
    this->textureMixerBuffer = createMultiLayerRenderFrameBuffer(width,height, 3, 3, false, GL_RGB8);
    this->program = program;
    this->blitter = blitter;
    this->previewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->layers = layers; 
}

TextureArray TextureMixer::getTexture(){
    return textureMixerBuffer.colorTexture;
}


MixerParams::MixerParams(int targetTexture, int baseTexture, int overlayTexture) {
    this->targetTexture = targetTexture;
    this->baseTexture = baseTexture;
    this->overlayTexture = overlayTexture;
}

void TextureMixer::mix(MixerParams &params){
    glUseProgram(program);
    glBindFramebuffer(GL_FRAMEBUFFER, textureMixerBuffer.frameBuffer);
    glViewport(0, 0, textureMixerBuffer.width, textureMixerBuffer.height);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    

    for(int i = 0; i < TEXTURE_TYPE_COUNT ; ++i) {
        glActiveTexture(GL_TEXTURE0+ i); 
        glBindTexture(GL_TEXTURE_2D_ARRAY, layers->textures[i].index);
        glUniform1i(glGetUniformLocation(program, ("textures[" + std::to_string(i) + "]").c_str()), i);
    }

    glUniform1ui(glGetUniformLocation(program, "baseTexture"), params.baseTexture);
    glUniform1ui(glGetUniformLocation(program, "overlayTexture"), params.overlayTexture);
    glUniform1i(glGetUniformLocation(program, "perlinScale"), params.perlinScale); 
    glUniform1f(glGetUniformLocation(program, "perlinTime"), params.perlinTime); 
    glUniform1i(glGetUniformLocation(program, "perlinIterations"), params.perlinIterations); 
    glUniform1i(glGetUniformLocation(program, "perlinLacunarity"), params.perlinLacunarity); 
    glUniform1f(glGetUniformLocation(program, "brightness"), params.brightness); 
    glUniform1f(glGetUniformLocation(program, "contrast"), params.contrast);
    glBindVertexArray(previewVao);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    for(int i=0 ; i < TEXTURE_TYPE_COUNT ; ++i ) {
        blitter->blit(&textureMixerBuffer, i, &layers->textures[i], params.targetTexture);
    }
    
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


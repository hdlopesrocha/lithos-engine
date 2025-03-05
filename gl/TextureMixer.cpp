#include "gl.hpp"

TextureMixer::TextureMixer(int width, int height, GLuint program, TextureLayers layers) {
    this->textureMixerBuffer = createMultiLayerRenderFrameBuffer(width,height, 3, false);
    this->baseTexture = createMultiLayerRenderFrameBuffer(width, height, 3, false);
    this->overlayTexture = createMultiLayerRenderFrameBuffer(width, height,3, false);
    this->program = program;
    this->previewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->perlinTime = 0;
    this->perlinScale = 16;
    this->perlinIterations = 5;
    this->perlinLacunarity = 2;
    this->brightness = 0;
    this->contrast = 10;
    this->colorTextures = layers.colorTextures; 
    this->normalTextures = layers.normalTextures;
    this->bumpTextures = layers.bumpTextures;
}

TextureArray TextureMixer::getTexture(){
    return textureMixerBuffer.colorTexture;
}

void TextureMixer::mix(int baseTextureIndex, int overlayTextureIndex, int index ){
    this->baseTextureIndex = baseTextureIndex;
    this->overlayTextureIndex = overlayTextureIndex;
    mix(index);
}

void TextureMixer::mix(int index){
    std::cout << "TextureMixer::mix" << std::endl;

    glUseProgram(program);
    glBindFramebuffer(GL_FRAMEBUFFER, textureMixerBuffer.frameBuffer);
    glViewport(0, 0, textureMixerBuffer.width, textureMixerBuffer.height);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    
    blitRenderBuffer(colorTextures, normalTextures, bumpTextures, baseTexture, baseTextureIndex);
    blitRenderBuffer(colorTextures, normalTextures, bumpTextures, overlayTexture, overlayTextureIndex);

    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D_ARRAY, baseTexture.colorTexture.index);
    glUniform1i(glGetUniformLocation(program, "baseTexture"), 0);

    glActiveTexture(GL_TEXTURE1); 
    glBindTexture(GL_TEXTURE_2D_ARRAY, overlayTexture.colorTexture.index);
    glUniform1i(glGetUniformLocation(program, "overlayTexture"), 1);

    glUniform1i(glGetUniformLocation(program, "perlinScale"), perlinScale); 
    glUniform1f(glGetUniformLocation(program, "perlinTime"), perlinTime); 
    glUniform1i(glGetUniformLocation(program, "perlinIterations"), perlinIterations); 
    glUniform1i(glGetUniformLocation(program, "perlinLacunarity"), perlinLacunarity); 
    glUniform1f(glGetUniformLocation(program, "brightness"), brightness); 
    glUniform1f(glGetUniformLocation(program, "contrast"), contrast);
    glBindVertexArray(previewVao);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureMixerBuffer.colorTexture.index);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    blitTextureArray(textureMixerBuffer, colorTextures, normalTextures, bumpTextures, index);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

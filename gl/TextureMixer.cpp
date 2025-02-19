#include "gl.hpp"

TextureMixer::TextureMixer(int width, int height, GLuint program, std::vector<Texture*> * textures) {
    this->textureMixerBuffer = createMultiLayerRenderFrameBuffer(width,height, 3);
    this->textures = textures;
    this->program = program;
    this->previewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->perlinTime = 0;
    this->perlinScale = 16;
    this->perlinIterations = 5;
    this->perlinLacunarity = 2;
    this->brightness = 0;
    this->contrast = 10;
}

TextureArray TextureMixer::getTexture(){
    return textureMixerBuffer.colorTexture;
}

void TextureMixer::mix(int baseTextureIndex, int overlayTextureIndex ){
    this->baseTextureIndex = baseTextureIndex;
    this->overlayTextureIndex = overlayTextureIndex;
    mix();
}

void TextureMixer::mix(){
    glUseProgram(program);
    glBindFramebuffer(GL_FRAMEBUFFER, textureMixerBuffer.frameBuffer);
    glViewport(0, 0, textureMixerBuffer.width, textureMixerBuffer.height);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    if(textures->size() > 0) {
        Texture * baseTexture = textures->at(Math::mod(baseTextureIndex, textures->size()));
        Texture * overlayTexture = textures->at(Math::mod(overlayTextureIndex, textures->size()));

        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D_ARRAY, baseTexture->texture);
        glUniform1i(glGetUniformLocation(program, "baseTexture"), 0);

        glActiveTexture(GL_TEXTURE1); 
        glBindTexture(GL_TEXTURE_2D_ARRAY, overlayTexture->texture);
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
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureMixerBuffer.colorTexture);
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    } else {
        std::cerr << "No textures in TextureMixer!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

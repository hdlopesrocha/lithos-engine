#include "gl.hpp"

TextureMixer::TextureMixer(int width, int height, GLuint program) {
    this->textureMixerBuffer = createMultiLayerRenderFrameBuffer(width,height, 3);
    this->textures = textures;
    this->program = program;
    this->previewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->perlinTime = 0;
    this->perlinScale = 10;
    this->perlinIterations = 10;
    this->brightness = 4;
    this->contrast = 10;
    
}

TextureArray TextureMixer::getTexture(){
    return textureMixerBuffer.texture;
}

void TextureMixer::mix(Texture * baseTexture, Texture * overlayTexture ){
    GLint originalFrameBuffer;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, textureMixerBuffer.frameBuffer);
    glViewport(0, 0, 1024, 1024); //TODO: from texture
    glUseProgram(program);

    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D_ARRAY, baseTexture->texture);
    glUniform1i(glGetUniformLocation(program, "baseTexture"), 0);


    glActiveTexture(GL_TEXTURE1); 
    glBindTexture(GL_TEXTURE_2D_ARRAY, overlayTexture->texture);
    glUniform1i(glGetUniformLocation(program, "overlayTexture"), 1);

    glUniform1f(glGetUniformLocation(program, "perlinScale"), perlinScale); // Set the sampler uniform
    glUniform1f(glGetUniformLocation(program, "perlinTime"), perlinTime); // Set the sampler uniform
    glUniform1i(glGetUniformLocation(program, "perlinIterations"), perlinIterations); // Set the sampler uniform
    glUniform1f(glGetUniformLocation(program, "brightness"), brightness); // Set the sampler uniform
    glUniform1f(glGetUniformLocation(program, "contrast"), contrast); // Set the sampler uniform

    for (int layer = 0; layer < 3; ++layer) {
        glUniform1i(glGetUniformLocation(program, "layerIndex"), layer);
        glBindVertexArray(previewVao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originalFrameBuffer);

}

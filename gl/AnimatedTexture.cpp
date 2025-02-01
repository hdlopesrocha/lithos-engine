#include "gl.hpp"

AnimatedTexture::AnimatedTexture(int width, int height, GLuint program, std::vector<Texture*> * textures) {
    this->textureMixerBuffer = createMultiLayerRenderFrameBuffer(width,height, 3);
    this->textures = textures;
    this->program = program;
    this->previewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->perlinScale = 16;
    this->perlinIterations = 5;
    this->perlinLacunarity = 2;
    this->brightness = 0;
    this->contrast = 1;
    
}

TextureArray AnimatedTexture::getTexture(){
    return textureMixerBuffer.texture;
}

void AnimatedTexture::animate(float time){


    GLint originalFrameBuffer;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);

    glUseProgram(program);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, textureMixerBuffer.frameBuffer);
    glViewport(0, 0, textureMixerBuffer.width, textureMixerBuffer.height);
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1i(glGetUniformLocation(program, "perlinScale"), perlinScale); // Set the sampler uniform
    glUniform1f(glGetUniformLocation(program, "perlinTime"), time); // Set the sampler uniform
    glUniform1i(glGetUniformLocation(program, "perlinIterations"), perlinIterations); // Set the sampler uniform
    glUniform1i(glGetUniformLocation(program, "perlinLacunarity"), perlinLacunarity); // Set the sampler uniform
    glUniform1f(glGetUniformLocation(program, "brightness"), brightness); // Set the sampler uniform
    glUniform1f(glGetUniformLocation(program, "contrast"), contrast); // Set the sampler uniform

    for (int layer = 0; layer < 3; ++layer) {
        glUniform1i(glGetUniformLocation(program, "layerIndex"), layer);
        glBindVertexArray(previewVao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureMixerBuffer.texture);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originalFrameBuffer);

}

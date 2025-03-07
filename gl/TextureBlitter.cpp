#include "gl.hpp"


TextureBlitter::TextureBlitter(GLuint program){
    this->program = program;
    this->resizeVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
}

void TextureBlitter::blit(MultiLayerRenderBuffer * sourceBuffer, int sourceIndex, TextureArray * targetBuffer, int targetIndex) {
    /*glUseProgram(program);
    glBindFramebuffer(GL_FRAMEBUFFER, targetBuffer->frameBuffer);
    glViewport(0, 0, targetBuffer->width, targetBuffer->height);

    // Set uniforms: source layer, sizes, etc.
    glUniform1i(glGetUniformLocation(program, "sourceLayer"), sourceIndex);
    glUniform1i(glGetUniformLocation(program, "targetLayer"), targetIndex);
   
    // Bind the source texture array to the sampler unit (assume it's unit 0)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, sourceBuffer->colorTexture.index);
    glUniform1i(glGetUniformLocation(program, "textureArray"), 0);

    glBindVertexArray(resizeVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, targetBuffer->colorTexture.index);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);


    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    */
   glCopyImageSubData(
    sourceBuffer->colorTexture.index, GL_TEXTURE_2D_ARRAY, 0, 0, 0, sourceIndex, // Source
    targetBuffer->index, GL_TEXTURE_2D_ARRAY, 0, 0, 0, targetIndex, // Destination
    sourceBuffer->width, sourceBuffer->height, 1 // Copy a single layer
    );

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, targetBuffer->index);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}


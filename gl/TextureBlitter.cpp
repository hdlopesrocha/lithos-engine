#include "gl.hpp"


TextureBlitter::TextureBlitter(GLuint program, int width, int height){
    this->program = program;
    this->resizeVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->bufferRGB8 = createMultiLayerRenderFrameBuffer(width, height, 1,1, false, GL_RGB8);
    this->bufferR8 = createMultiLayerRenderFrameBuffer(width, height, 1,1, false, GL_R8);
}

void TextureBlitter::blit(MultiLayerRenderBuffer * sourceBuffer, int sourceIndex, TextureArray * targetBuffer, int targetIndex) {
    MultiLayerRenderBuffer * buffer = targetBuffer->channel == GL_RGB8 ? &bufferRGB8 : &bufferR8;
    
    glUseProgram(program);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer->frameBuffer);
    glViewport(0, 0, buffer->width, buffer->height);

    // Set uniforms: source layer, sizes, etc.
    glUniform1i(glGetUniformLocation(program, "sourceLayer"), sourceIndex);
    glUniform1i(glGetUniformLocation(program, "targetLayer"), 0);
   
    // Bind the source texture array to the sampler unit (assume it's unit 0)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, sourceBuffer->colorTexture.index);
    glUniform1i(glGetUniformLocation(program, "textureArray"), 0);

    glBindVertexArray(resizeVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, buffer->colorTexture.index);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);


    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glCopyImageSubData(
        buffer->colorTexture.index, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, // Source
        targetBuffer->index, GL_TEXTURE_2D_ARRAY, 0, 0, 0, targetIndex, // Destination
        buffer->width, buffer->height, 1 // Copy a single layer
    );

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, targetBuffer->index);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}


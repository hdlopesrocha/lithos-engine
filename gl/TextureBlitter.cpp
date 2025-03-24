#include "gl.hpp"


TextureBlitter::TextureBlitter(GLuint program, int width, int height, std::initializer_list<GLuint> formats){
    this->program = program;
    this->resizeVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);

    for(GLuint format : formats) {
        buffers.insert({format, createMultiLayerRenderFrameBuffer(width, height, 3,3, false, format)});
    }
}

void TextureBlitter::blit(MultiLayerRenderBuffer * sourceBuffer, int sourceIndex, TextureArray * targetBuffer, int targetIndex) {
    MultiLayerRenderBuffer * buffer = &buffers[targetBuffer->format];
    if(buffer != NULL) {
        glBindFramebuffer(GL_FRAMEBUFFER, buffer->frameBuffer);
        glViewport(0, 0, buffer->width, buffer->height);
        glClearColor(0.0f,1.0f,0.0f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);

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
    } else {
        std::cerr << "Blit error: " << targetBuffer->format << " channel not mapped!" << std::endl;
    }
}


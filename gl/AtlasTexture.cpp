#include "gl.hpp"

AtlasTexture::AtlasTexture(int width, int height, GLuint program, std::vector<Texture*> * textures, int selectedTexture, std::vector<Tile> tiles) {
    this->textureBuffer = createMultiLayerRenderFrameBuffer(width,height, 3);
    this->textures = textures;
    this->selectedTexture = selectedTexture;    
    this->program = program;
    this->previewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->samplerLoc = glGetUniformLocation(program, "textureSampler");
    this->layerLoc = glGetUniformLocation(program, "layerIndex");
    this->modelLoc = glGetUniformLocation(program, "model");
    this->tileOffsetLoc = glGetUniformLocation(program, "tileOffset");
    this->tileSizeLoc = glGetUniformLocation(program, "tileSize");
    this->tiles = tiles;
}

TextureArray AtlasTexture::getTexture(){
    return textureBuffer.colorTexture;
}

void AtlasTexture::mix(){
    GLint originalFrameBuffer;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);
    Texture * texture = (*this->textures)[this->selectedTexture];

    glUseProgram(program);

    glm::mat3 model = glm::rotate(glm::scale(glm::mat4(1.0), glm::vec3(0.5)), (float) (M_PI/4.0), glm::vec3(0.0,0.0,1.0) );
    glm::vec2 tileOffset = glm::vec2(0.5);
    glm::vec2 tileSize = glm::vec2(0.25);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, textureBuffer.frameBuffer);
    glViewport(0, 0, textureBuffer.width, textureBuffer.height);
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture->texture);
    glUniform1i(this->samplerLoc, 0);
	glUniformMatrix3fv(this->modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform2fv(this->tileOffsetLoc, 1, glm::value_ptr(tileOffset));
	glUniform2fv(this->tileSizeLoc, 1, glm::value_ptr(tileSize));

    for (int layer = 0; layer < 3; ++layer) {
        glUniform1i(this->layerLoc, layer);
        glBindVertexArray(previewVao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureBuffer.colorTexture);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originalFrameBuffer);
}

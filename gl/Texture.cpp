#include "gl.hpp"

// type = GL_TEXTURE_2D_ARRAY || GL_TEXTURE_2D

int bindTextureInternal(GLuint program, GLuint type, int activeTexture, GLuint location, GLuint texture) {
    glUseProgram(program);

    glActiveTexture(GL_TEXTURE0 + activeTexture); 
    glBindTexture(type, texture);    // Bind the texture to the active unit
    glUniform1i(location, activeTexture++);

    return activeTexture;
}

int bindTextureInternal(GLuint program, GLuint type, int activeTexture, std::string objectName, GLuint texture) {
    glUseProgram(program);

    glActiveTexture(GL_TEXTURE0 + activeTexture); 
    glBindTexture(type, texture);    // Bind the texture to the active unit
    glUniform1i(glGetUniformLocation(program, objectName.c_str()), activeTexture++);

    return activeTexture;
}

int Texture::bindTexture(GLuint program, int activeTexture, GLuint location, TextureArray texture) {
    return bindTextureInternal(program, GL_TEXTURE_2D_ARRAY, activeTexture, location, texture.index);
}

int Texture::bindTexture(GLuint program, int activeTexture, std::string objectName, TextureArray texture) {
    return bindTextureInternal(program, GL_TEXTURE_2D_ARRAY, activeTexture, objectName, texture.index);
}

int Texture::bindTexture(GLuint program, int activeTexture, std::string objectName, TextureImage texture) {
    return bindTextureInternal(program, GL_TEXTURE_2D, activeTexture, objectName, texture.index);
}

int Texture::bindTexture(GLuint program, int activeTexture, GLuint location, TextureImage texture) {
    return bindTextureInternal(program, GL_TEXTURE_2D, activeTexture, location, texture.index);
}
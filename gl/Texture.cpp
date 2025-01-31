#include "gl.hpp"

Texture::Texture() {
    this->texture = 0;
}

Texture::Texture(TextureArray texture) : Texture() {
    this->texture = texture;
}
// type = GL_TEXTURE_2D_ARRAY || GL_TEXTURE_2D
int Texture::bindTexture(GLuint program, GLuint type, int activeTexture, std::string objectName, GLuint texture) {
    glUseProgram(program);

    glActiveTexture(GL_TEXTURE0 + activeTexture); 
    glBindTexture(type, texture);    // Bind the texture to the active unit
    glUniform1i(glGetUniformLocation(program, objectName.c_str()), activeTexture++);

    return activeTexture;
}

int Texture::bindTextures(GLuint program, GLuint type,int activeTexture, std::string arrayName, std::vector<Texture*> * ts) {
    for(int i=0 ; i < ts->size() ; ++i) {
        std::string objectName = arrayName + "[" + std::to_string(i) + "]";
        Texture * t = (*ts)[i];
        t->index = i;
        activeTexture = Texture::bindTexture(program, type, activeTexture, objectName, t->texture);
    }
    return activeTexture;
}
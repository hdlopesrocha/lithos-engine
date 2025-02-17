#include "gl.hpp"
#include <glm/gtc/type_ptr.hpp>

AnimatedTexture::AnimatedTexture(int width, int height, GLuint program) {
    this->textureMixerBuffer = createMultiLayerRenderFrameBuffer(width,height, 3);
    this->program = program;
    this->previewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->perlinScale = 9;
    this->perlinIterations = 8;
    this->perlinLacunarity = 3;
    this->brightness = 0;
    this->contrast = 1;
    this->color = glm::vec4(64/255.0f, 64/255.0f, 64/255.0f, 1.0);
}

TextureArray AnimatedTexture::getTexture(){
    return textureMixerBuffer.colorTexture;
}

void AnimatedTexture::animate(float time){
    glUseProgram(program);

    glBindFramebuffer(GL_FRAMEBUFFER, textureMixerBuffer.frameBuffer);
    glViewport(0, 0, textureMixerBuffer.width, textureMixerBuffer.height);
	glClearColor (0.0,0.0,0.0,0.0);    
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUniform1i(glGetUniformLocation(program, "perlinScale"), perlinScale); // Set the sampler uniform
    glUniform1f(glGetUniformLocation(program, "perlinTime"), time); // Set the sampler uniform
    glUniform1i(glGetUniformLocation(program, "perlinIterations"), perlinIterations); // Set the sampler uniform
    glUniform1i(glGetUniformLocation(program, "perlinLacunarity"), perlinLacunarity); // Set the sampler uniform
    glUniform1f(glGetUniformLocation(program, "brightness"), brightness); // Set the sampler uniform
    glUniform1f(glGetUniformLocation(program, "contrast"), contrast); // Set the sampler uniform
    glUniform4fv(glGetUniformLocation(program, "color"), 1, glm::value_ptr(color)); // Set the sampler uniform
    glBindVertexArray(previewVao);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureMixerBuffer.colorTexture);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

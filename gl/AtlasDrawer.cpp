#include "gl.hpp"


AtlasDrawer::AtlasDrawer(GLuint program, int width, int height) {
    this->program = program;
    this->renderBuffer = createMultiLayerRenderFrameBuffer(width,height, 3);
    this->viewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->samplerLoc = glGetUniformLocation(program, "textureSampler");
    this->modelLoc = glGetUniformLocation(program, "model");
    this->tileOffsetLoc = glGetUniformLocation(program, "tileOffset");
    this->tileSizeLoc = glGetUniformLocation(program, "tileSize");
}

TextureArray AtlasDrawer::getTexture(){
    return renderBuffer.colorTexture;
}

void AtlasDrawer::draw(AtlasTexture * atlas,  std::vector<TileDraw> draws){
    glUseProgram(program);

    glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer.frameBuffer);
    glViewport(0, 0, renderBuffer.width, renderBuffer.height); 
	glClearColor (0.0,0.0,0.0,0.0);    
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D_ARRAY, atlas->texture);
    glUniform1i(samplerLoc, 0); // Set the sampler uniform
    glBindVertexArray(viewVao);
    
    for(int i=0 ; i < draws.size() ; ++i) {
        TileDraw tileDraw = draws[i];
       // std::cout << tileDraw.index << " -> "<< tileDraw.offset.x << ":" <<tileDraw.offset.y << "[" << tileDraw.size.x << "," << tileDraw.size.y << "]" << std::endl;        
        Tile * tile = &atlas->tiles[tileDraw.index];

        glm::mat4 model = glm::mat4(1.0);

        // 3 - transform from [0:1 to -1:1]
        model = glm::translate(model, glm::vec3(-1.0));
        model = glm::scale(model, glm::vec3(2.0));

        // 2 - scale + translate
        model = glm::translate(model, glm::vec3(tileDraw.offset, 0.0));
        model = glm::scale(model, glm::vec3(tileDraw.size, 1.0));

        // 1 - transform from [-1:1 to 0:1]
        model = glm::scale(model, glm::vec3(0.5));
        model = glm::translate(model, glm::vec3(1.0));

        glUniform2fv(tileOffsetLoc, 1, glm::value_ptr(tile->offset));
        glUniform2fv(tileSizeLoc, 1, glm::value_ptr(tile->size));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
   
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}




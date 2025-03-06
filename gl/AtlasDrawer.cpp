#include "gl.hpp"


AtlasDrawer::AtlasDrawer(GLuint program, int width, int height , TextureLayers * sourceLayers, TextureLayers * targetLayers) {
    this->program = program;
    this->sourceLayers = sourceLayers;
    this->targetLayers = targetLayers;
    this->renderBuffer = createMultiLayerRenderFrameBuffer(width,height, 3, false, GL_RGB8);
    this->viewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->atlasTextureLoc = glGetUniformLocation(program, "atlasTexture");
    this->modelLoc = glGetUniformLocation(program, "model");
    this->tileOffsetLoc = glGetUniformLocation(program, "tileOffset");
    this->tileSizeLoc = glGetUniformLocation(program, "tileSize");
    this->filterLoc = glGetUniformLocation(program, "filterOpacity");
}

TextureArray AtlasDrawer::getTexture(){
    return renderBuffer.colorTexture;
}

AtlasParams::AtlasParams(){

};

 AtlasParams::AtlasParams(int sourceTexture, int targetTexture, AtlasTexture * atlasTexture){
    this->targetTexture = targetTexture;
    this->sourceTexture = sourceTexture;
    this->atlasTexture = atlasTexture;
 }

void AtlasDrawer::draw(AtlasParams params){

    glUseProgram(program);

    glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer.frameBuffer);
    glViewport(0, 0, renderBuffer.width, renderBuffer.height); 
    glClearColor (0.0,0.0,0.0,1.0);    
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
        

    for(int i = 0; i < 3 ; ++i) {
        glActiveTexture(GL_TEXTURE0+ i); 
        glBindTexture(GL_TEXTURE_2D_ARRAY, sourceLayers->textures[i].index);
        glUniform1i(glGetUniformLocation(program, ("sampler[" + std::to_string(i) + "]").c_str()), i);
    }

    glUniform1ui(filterLoc, filterEnabled); 
    glUniform1ui(atlasTextureLoc, params.sourceTexture); 
    glBindVertexArray(viewVao);
    
    for(TileDraw &tileDraw : params.draws) {
        uint tileIndex = Math::mod(tileDraw.index, params.atlasTexture->tiles.size());


        Tile * tile = &params.atlasTexture->tiles[tileIndex];

        glm::mat4 model = glm::mat4(1.0);

        // 3 - transform from [0:1 to -1:1]
        model = glm::translate(model, glm::vec3(-1.0));
        model = glm::scale(model, glm::vec3(2.0));

        // 2 - scale + translate + rotate
        model = glm::translate(model, glm::vec3(tileDraw.offset, 0.0));
        model = glm::rotate(model, tileDraw.angle, glm::vec3(0.0, 0.0, 1.0));
        model = glm::scale(model, glm::vec3(tileDraw.size, 1.0));
        model = glm::translate(model, -glm::vec3(tileDraw.pivot, 0.0f));


        // 1 - transform from [-1:1 to 0:1]
        model = glm::scale(model, glm::vec3(0.5));
        model = glm::translate(model, glm::vec3(1.0));

        glUniform2fv(tileOffsetLoc, 1, glm::value_ptr(tile->offset));
        glUniform2fv(tileSizeLoc, 1, glm::value_ptr(tile->size));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    }

    blitTextureArray(renderBuffer, *targetLayers, params.targetTexture);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}





#include "gl.hpp"


AtlasDrawer::AtlasDrawer(GLuint program, int width, int height , TextureLayers * sourceLayers, TextureLayers * targetLayers, TextureBlitter * blitter) {
    this->program = program;
    this->sourceLayers = sourceLayers;
    this->targetLayers = targetLayers;
    this->renderBuffer = createMultiLayerRenderFrameBuffer(width,height, 3, 3, false, GL_RGB8);
    this->viewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->atlasTextureLoc = glGetUniformLocation(program, "atlasTexture");
    this->modelLoc = glGetUniformLocation(program, "model");
    this->tileOffsetLoc = glGetUniformLocation(program, "tileOffset");
    this->tileSizeLoc = glGetUniformLocation(program, "tileSize");
    this->filterLoc = glGetUniformLocation(program, "filterOpacity");
    this->blitter = blitter;

}


AtlasParams::AtlasParams(){

};

 AtlasParams::AtlasParams(int sourceTexture, int targetTexture){
    this->targetTexture = targetTexture;
    this->sourceTexture = sourceTexture;
 }

void AtlasDrawer::draw(AtlasParams &params, std::vector<AtlasTexture> *textures){

    glUseProgram(program);

    glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer.frameBuffer);
    glViewport(0, 0, renderBuffer.width, renderBuffer.height); 
    glClearColor (0.0,0.0,0.0,1.0);    
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
        

    for(int i = 0; i < TEXTURE_TYPE_COUNT ; ++i) {
        glActiveTexture(GL_TEXTURE0+ i); 
        glBindTexture(GL_TEXTURE_2D_ARRAY, sourceLayers->textures[i].index);
        glUniform1i(glGetUniformLocation(program, ("textures[" + std::to_string(i) + "]").c_str()), i);
    }

    glUniform1ui(filterLoc, filterEnabled); 
    glUniform1ui(atlasTextureLoc, params.sourceTexture); 
    glBindVertexArray(viewVao);
    
    for(TileDraw &tileDraw : params.draws) {
        AtlasTexture * atlas = &(*textures)[params.sourceTexture];
        uint tileIndex = Math::mod(tileDraw.index, atlas->tiles.size());


        Tile * tile = &atlas->tiles[tileIndex];

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

    for(int i =0 ; i < TEXTURE_TYPE_COUNT; ++i) {
        blitter->blit(&renderBuffer, i, &targetLayers->textures[i], params.targetTexture);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}





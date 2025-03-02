#include "gl.hpp"


AtlasDrawer::AtlasDrawer(GLuint program, int width, int height, std::vector<AtlasTexture*> * atlasTextures) {
    this->program = program;
    this->atlasTextures = atlasTextures;
    this->renderBuffer = createMultiLayerRenderFrameBuffer(width,height, 4);
    this->viewVao = DrawableGeometry::create2DVAO(-1,-1, 1,1);
    this->samplerLoc = glGetUniformLocation(program, "textureSampler");
    this->modelLoc = glGetUniformLocation(program, "model");
    this->tileOffsetLoc = glGetUniformLocation(program, "tileOffset");
    this->tileSizeLoc = glGetUniformLocation(program, "tileSize");
    this->filterLoc = glGetUniformLocation(program, "filterOpacity");
    this->atlasIndex = 0;
}

TextureArray AtlasDrawer::getTexture(){
    return renderBuffer.colorTexture;
}

void AtlasDrawer::draw(int atlasIndex,  std::vector<TileDraw> draws){
    this->atlasIndex = atlasIndex;
    this->draws = draws;
    this->draw();
}

void AtlasDrawer::draw(){
    glUseProgram(program);

    glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer.frameBuffer);
    glViewport(0, 0, renderBuffer.width, renderBuffer.height); 
    glClearColor (0.0,0.0,0.0,1.0);    
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    if(atlasTextures->size() > 0) {
        AtlasTexture * atlas = atlasTextures->at(Math::mod(atlasIndex, atlasTextures->size()));
        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D_ARRAY, atlas->texture);
        glUniform1i(samplerLoc, 0); 
        glUniform1ui(filterLoc, filterEnabled); 
        glBindVertexArray(viewVao);
        
        for(TileDraw tileDraw : draws) {
            uint tileIndex = Math::mod(tileDraw.index, atlas->tiles.size());

           /* std::cout << tileDraw.index << " -> " <<
                    "o={"<< tileDraw.offset.x << ":" <<tileDraw.offset.y << 
                    "},s={" << tileDraw.size.x << ":" << tileDraw.size.y << 
                    "},p={" << tileDraw.pivot.x << ":" << tileDraw.pivot.y << 
                    "},a=" << tileDraw.angle << std::endl;    */    
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

        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D_ARRAY, renderBuffer.colorTexture);
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    } else {
        std::cerr << "No textures in AtlasDrawer!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}




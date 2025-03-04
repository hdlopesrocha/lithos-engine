#include "gl.hpp"

ImpostorDrawer::ImpostorDrawer(GLuint program, int width, int height) {
    this->height = height;
    this->width = width;
    this->program = program;
    this->renderBuffer = createMultiLayerRenderFrameBuffer(width,height, 4);

    std::vector<InstanceData> vegetationInstances;
    vegetationInstances.push_back(InstanceData(glm::mat4(1.0), 0));
    this->mesh = new DrawableInstanceGeometry(new Vegetation3d(), &vegetationInstances, glm::vec3(0.0, 0.5, 0.0));
}

TextureArray ImpostorDrawer::getTexture() {
    return renderBuffer.colorTexture;
}

void ImpostorDrawer::draw(ProgramData * programData, UniformBlock uniformBlock) {
    glm::mat4 view = glm::lookAt(glm::vec3(2.0), mesh->center, glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), width / (float) height, 0.1f, 32.0f);
    glm::mat4 viewProjection = projection * view;

    uniformBlock.viewProjection = viewProjection;

    glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer.frameBuffer);
    glViewport(0, 0, renderBuffer.width, renderBuffer.height);
    glClearColor (0.0,0.0,0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);




    programData->uniform(&uniformBlock);
    glDisable(GL_CULL_FACE);
    mesh->draw(GL_TRIANGLES);
    glEnable(GL_CULL_FACE);
}
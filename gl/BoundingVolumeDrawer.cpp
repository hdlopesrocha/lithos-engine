#include "gl.hpp"

BoundingVolumeDrawer::BoundingVolumeDrawer(DrawableInstanceGeometry<InstanceData> * sphereGeometry, DrawableInstanceGeometry<InstanceData> * boxGeometry, GLuint program, ProgramData &data, UniformBlock uniformBlock, UniformBlockBrush uniformBrush):
    program(program),
    data(data),
    uniformBlock(uniformBlock),
    uniformBrush(uniformBrush)
 {
    this->sphereGeometry = sphereGeometry;
    this->boxGeometry = boxGeometry;
}

BoundingVolumeDrawer::~BoundingVolumeDrawer() {

}

void BoundingVolumeDrawer::bind(glm::mat4 model) {
    glUseProgram(program);
    uniformBlock.world = model;
    uniformBlock.set(OVERRIDE_FLAG, true);
    uniformBlock.uintData.w = uint(0);
    UniformBlock::uniform(0, &uniformBlock, sizeof(UniformBlock), data);
    UniformBlockBrush::uniform(program, &uniformBrush, "overrideProps");
}

void BoundingVolumeDrawer::visit(const BoundingSphere& sphere) {
    long count = 0;
    glm::mat4 model = glm::scale(
        glm::translate(  
            glm::mat4(1.0f), 
            sphere.center
        ), 
        glm::vec3(sphere.radius)
    );
    bind(model);
    sphereGeometry->draw(GL_PATCHES, &count);
};
    

void BoundingVolumeDrawer::visit(const AbstractBoundingBox& box) {
    long count = 0;
    glm::mat4 model = glm::scale(
        glm::translate(  
            glm::mat4(1.0f), 
            box.getMin()
        ), 
        box.getLength()
    );
    bind(model);
    boxGeometry->draw(GL_PATCHES, &count);
};

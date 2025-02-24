
#include "gl.hpp"

glm::vec3 rotate(glm::vec3 point, glm::vec3 axis, float angle) {
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0), angle, axis);
    glm::vec4 result = rotationMatrix * glm::vec4(point, 1.0f);
    return result;
}

glm::vec3 rotate2(glm::vec3 point, glm::vec3 axis1, float angle1, glm::vec3 axis2, float angle2) {
    glm::mat4 rot = glm::rotate(glm::rotate(glm::mat4(1.0), angle1, axis1), angle2, axis2);
    glm::vec4 result = rot * glm::vec4(point, 1.0f);
    return result;
}

Vegetation3d::Vegetation3d() : Geometry(){
    int planes = 4;
        float PI = glm::pi<float>();

    glm::vec3 axis1 = glm::vec3(0,1,0);
    for (int i = 0; i < planes; ++i) {
        float angle = (i / (float) planes)* PI;

        addVertex(Vertex(rotate(glm::vec3(-0.5,0,0), axis1, angle), glm::vec3(0,1,0), glm::vec2(1,1),2), false );
        addVertex(Vertex(rotate(glm::vec3(-0.5,1,0), axis1, angle), glm::vec3(0,1,0), glm::vec2(1,0),2), false );
        addVertex(Vertex(rotate(glm::vec3(0.5,1,0), axis1, angle), glm::vec3(0,1,0), glm::vec2(0,0),2), false );

        addVertex(Vertex(rotate(glm::vec3(-0.5,0,0), axis1, angle), glm::vec3(0,1,0), glm::vec2(1,1),2), false );
        addVertex(Vertex(rotate(glm::vec3(0.5,0,0), axis1, angle), glm::vec3(0,1,0), glm::vec2(0,1),2), false );
        addVertex(Vertex(rotate(glm::vec3(0.5,1,0), axis1, angle), glm::vec3(0,1,0), glm::vec2(0,0),2), false );
    }

    glm::vec3 axis2 = glm::vec3(1,0,0);
    for (int i = 0; i < planes; ++i) {
        float angle = (i / (float) planes)* PI * 2;

        addVertex(Vertex(rotate2(glm::vec3(-0.5,0,0), axis1, angle, axis2, PI/4), glm::vec3(0,1,0), glm::vec2(1,1),2), false );
        addVertex(Vertex(rotate2(glm::vec3(-0.5,1,0), axis1, angle, axis2, PI/4), glm::vec3(0,1,0), glm::vec2(1,0),2), false );
        addVertex(Vertex(rotate2(glm::vec3(0.5,1,0), axis1, angle, axis2, PI/4), glm::vec3(0,1,0), glm::vec2(0,0),2), false );

        addVertex(Vertex(rotate2(glm::vec3(-0.5,0,0), axis1, angle, axis2, PI/4), glm::vec3(0,1,0), glm::vec2(1,1),2), false );
        addVertex(Vertex(rotate2(glm::vec3(0.5,0,0), axis1, angle, axis2, PI/4), glm::vec3(0,1,0), glm::vec2(0,1),2), false );
        addVertex(Vertex(rotate2(glm::vec3(0.5,1,0), axis1, angle, axis2, PI/4), glm::vec3(0,1,0), glm::vec2(0,0),2), false );
    }

}


DrawableInstanceGeometry * Vegetation3d::createDrawable(std::vector<glm::mat4> * instances) {
    return new DrawableInstanceGeometry(this, instances);
}

#include "gl.hpp"

Vegetation::Vegetation() {
    Geometry * geometry = new Geometry();
    int planes = 4;
        float PI = glm::pi<float>();

    for (int i = 0; i < planes; ++i) {
        float angle = (i / (float) planes)* PI;

        geometry->addVertex(Vertex(glm::vec3(0.5*glm::sin(angle),0,0.5*glm::cos(angle)), glm::vec3(0,1,0), glm::vec2(0,0),2), false );
        geometry->addVertex(Vertex(glm::vec3(0.5*glm::sin(angle),1,0.5*glm::cos(angle)), glm::vec3(0,1,0), glm::vec2(1,0),2), false );
        geometry->addVertex(Vertex(glm::vec3(0.5*glm::sin(angle+PI),1,0.5*glm::cos(angle+PI)), glm::vec3(0,1,0), glm::vec2(1,1),2), false );

        geometry->addVertex(Vertex(glm::vec3(0.5*glm::sin(angle),0,0.5*glm::cos(angle)), glm::vec3(0,1,0), glm::vec2(0,0),2), false );
        geometry->addVertex(Vertex(glm::vec3(0.5*glm::sin(angle+PI),0,0.5*glm::cos(angle+PI)), glm::vec3(0,1,0), glm::vec2(0,1),2), false );
        geometry->addVertex(Vertex(glm::vec3(0.5*glm::sin(angle+PI),1,0.5*glm::cos(angle+PI)), glm::vec3(0,1,0), glm::vec2(1,1),2), false );
    }


    this->drawable = new DrawableGeometry(geometry);
}


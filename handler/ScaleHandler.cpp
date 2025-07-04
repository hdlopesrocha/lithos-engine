#include "handler.hpp"

ScaleHandler::ScaleHandler(Camera &camera, glm::vec3 &vector) : camera(camera), vector(vector) {

}

void ScaleHandler::handle(Axis3dEvent value) {
    vector += value.deltaTime*value.axis;
    if(vector.x < 0.0f){ 
        vector.x = 0.0f;
    }
    if(vector.y < 0.0f){ 
        vector.y = 0.0f;
    }
    if(vector.z < 0.0f){ 
        vector.z = 0.0f;
    }
}


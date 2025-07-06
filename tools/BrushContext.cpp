#include "tools.hpp"

BrushContext::BrushContext(Camera *camera) : camera(camera) {
    this->simplifier = new Simplifier(0.99f, 0.01f, true);
    this->boundingVolume = BoundingSphere(glm::vec3(0), 3.0f);
    
    this->functions.push_back(new SphereDistanceFunction(glm::vec3(0), 1.0f));
    this->functions.push_back(new BoxDistanceFunction(glm::vec3(0), glm::vec3(1.0f)));
    this->functions.push_back(new CapsuleDistanceFunction(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), 1.0f));
    this->currentFunction = this->functions[0];
    this->detail = 1.0f;
}


void BrushContext::handleEvent(Event * event) {

    if(currentFunction == functions[0]) {
        SphereDistanceFunction * function = (SphereDistanceFunction*) currentFunction;
        if(event->getType() == EVENT_VECTOR_3D_0) {
            Axis3dEvent * e = (Axis3dEvent*) event;
            TranslateHandler(camera, &(function->center)).handle(e);
            std::cout << "EVENT_VECTOR_3D_0 = " << std::to_string(e->axis.x)<< ":"<< std::to_string(e->axis.y)<< ":"<<  std::to_string(e->axis.z) << std::endl;
        }
        if(event->getType() == EVENT_FLOAT_1_X) {
            FloatEvent * e = (FloatEvent*) event;
            FloatHandler(&(function->radius)).handle(e);
            std::cout << "EVENT_FLOAT_1_X = " << std::to_string(e->value) << std::endl;
        }
    }

    if(currentFunction == functions[1]) {
        BoxDistanceFunction * function = (BoxDistanceFunction*) currentFunction;
        if(event->getType() == EVENT_VECTOR_3D_0) {
            Axis3dEvent * e = (Axis3dEvent*) event;
            TranslateHandler(camera, &(function->center)).handle(e);
            std::cout << "EVENT_VECTOR_3D_0 = " << std::to_string(e->axis.x)<< ":"<< std::to_string(e->axis.y)<< ":"<<  std::to_string(e->axis.z) << std::endl;
        } 
        if(event->getType() == EVENT_FLOAT_1_X) {
            FloatEvent * e = (FloatEvent*) event;
            ScaleHandler(&(function->length.x)).handle(e);
            std::cout << "EVENT_FLOAT_1_X = " << std::to_string(e->value)<< std::endl;
        }
        if(event->getType() == EVENT_FLOAT_1_Y) {
            FloatEvent * e = (FloatEvent*) event;
            ScaleHandler(&(function->length.y)).handle(e);
            std::cout << "EVENT_FLOAT_1_Y = " << std::to_string(e->value)<< std::endl;
        }
        if(event->getType() == EVENT_FLOAT_1_Z) {
            FloatEvent * e = (FloatEvent*) event;
            ScaleHandler(&(function->length.z)).handle(e);
            std::cout << "EVENT_FLOAT_1_Z = " << std::to_string(e->value)<< std::endl;
        }
    }

    if(currentFunction == functions[2]) {
        CapsuleDistanceFunction * function = (CapsuleDistanceFunction*) currentFunction;
        if(event->getType() == EVENT_VECTOR_3D_0) {
            Axis3dEvent * e = (Axis3dEvent*) event;
            TranslateHandler(camera, &(function->a)).handle(e);
            std::cout << "EVENT_VECTOR_3D_0 = " << std::to_string(e->axis.x)<< ":"<< std::to_string(e->axis.y)<< ":"<<  std::to_string(e->axis.z) << std::endl;
        } 
        if(event->getType() == EVENT_VECTOR_3D_2) {
            Axis3dEvent * e = (Axis3dEvent*) event;
            TranslateHandler(camera, &(function->b)).handle(e);
            std::cout << "EVENT_VECTOR_3D_2 = " << std::to_string(e->axis.x)<< ":"<< std::to_string(e->axis.y)<< ":"<<  std::to_string(e->axis.z) << std::endl;
        }
        if(event->getType() == EVENT_FLOAT_1_X) {
            FloatEvent * e = (FloatEvent*) event;
            FloatHandler(&(function->radius)).handle(e);
            std::cout << "EVENT_FLOAT_1_X = " << std::to_string(e->value) << std::endl;
        }
    }


}
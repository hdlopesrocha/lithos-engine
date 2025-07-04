#include "tools.hpp"
#include "../command/command.hpp"

BrushContext::BrushContext(Camera &camera) : camera(camera) {
    this->simplifier = new Simplifier(0.99f, 0.01f, true);
    this->boundingVolume = BoundingSphere(glm::vec3(0), 3.0f);
    {
        SphereDistanceFunction * function = new SphereDistanceFunction(glm::vec3(0), 1.0f);   
        this->functions.push_back(function);
    }
    {
        BoxDistanceFunction * function = new BoxDistanceFunction(glm::vec3(0), glm::vec3(1.0f));
        this->functions.push_back(function);
    }
    {
        CapsuleDistanceFunction * function = new CapsuleDistanceFunction(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), 1.0f);
        this->functions.push_back(function);
    }

    this->currentFunction = this->functions[0];
    this->detail = 1.0f;
}


void BrushContext::handleEvent(Event &event) {

    if(currentFunction == functions[0]) {
        SphereDistanceFunction * function = (SphereDistanceFunction*) currentFunction;
        if(event.getType() == EVENT_VECTOR_3D_0) {
            Axis3dEvent * axisEvent = (Axis3dEvent*) &event;
            TranslateCommand(camera, function->center).execute(*axisEvent);
        } 
    }

    if(currentFunction == functions[1]) {
        BoxDistanceFunction * function = (BoxDistanceFunction*) currentFunction;
        if(event.getType() == EVENT_VECTOR_3D_0) {
            Axis3dEvent * axisEvent = (Axis3dEvent*) &event;
            TranslateCommand(camera, function->center).execute(*axisEvent);
        } 
        if(event.getType() == EVENT_VECTOR_3D_1) {
            Axis3dEvent * axisEvent = (Axis3dEvent*) &event;
            ScaleCommand(camera, function->length).execute(*axisEvent);
        }
    }

    if(currentFunction == functions[2]) {
        CapsuleDistanceFunction * function = (CapsuleDistanceFunction*) currentFunction;
        if(event.getType() == EVENT_VECTOR_3D_0) {
            Axis3dEvent * axisEvent = (Axis3dEvent*) &event;
            TranslateCommand(camera, function->a).execute(*axisEvent);
        } 
        if(event.getType() == EVENT_VECTOR_3D_1) {
            Axis3dEvent * axisEvent = (Axis3dEvent*) &event;
            TranslateCommand(camera, function->b).execute(*axisEvent);
        }
    }


}
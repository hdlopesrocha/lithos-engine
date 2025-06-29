#include "tools.hpp"

BrushContext::BrushContext() {
    this->simplifier = new Simplifier(0.99f, 0.01f, true);
    this->boundingVolume = BoundingSphere(glm::vec3(0), 3.0f);
    {
        SphereDistanceFunction * function = new SphereDistanceFunction(glm::vec3(0), 1.0f);   
        this->functions.push_back(function);
        ControlledObject * controlledFunction = new ControlledObject();
        controlledFunction->attributes.push_back(new ControlledAttribute<glm::vec3>(&function->center, TYPE_VEC3) );
        controlledFunction->attributes.push_back(new ControlledAttribute<float>(&function->radius, TYPE_FLOAT) );
        this->controlledObjects.push_back(controlledFunction);
    }
    {
        BoxDistanceFunction * function = new BoxDistanceFunction(glm::vec3(0), glm::vec3(1.0f));
        this->functions.push_back(function);
        ControlledObject * controlledFunction = new ControlledObject();
        controlledFunction->attributes.push_back(new ControlledAttribute<glm::vec3>(&function->center, TYPE_VEC3) );
        controlledFunction->attributes.push_back(new ControlledAttribute<glm::vec3>(&function->length, TYPE_VEC3) );
        this->controlledObjects.push_back(controlledFunction);
    }
    {
        CapsuleDistanceFunction * function = new CapsuleDistanceFunction(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), 1.0f);
        this->functions.push_back(function);
        ControlledObject * controlledFunction = new ControlledObject();
        controlledFunction->attributes.push_back(new ControlledAttribute<glm::vec3>(&function->a, TYPE_VEC3) );
        controlledFunction->attributes.push_back(new ControlledAttribute<glm::vec3>(&function->b, TYPE_VEC3) );
        controlledFunction->attributes.push_back(new ControlledAttribute<float>(&function->radius, TYPE_FLOAT) );
        this->controlledObjects.push_back(controlledFunction);
    }

    this->currentFunction = this->functions[0];
    this->detail = 1.0f;
}
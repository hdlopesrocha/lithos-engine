#include "tools.hpp"

BrushContext::BrushContext(Camera *camera) : camera(camera) {
    this->simplifier = new Simplifier(0.99f, 0.01f, true);
    this->boundingVolume = BoundingSphere(glm::vec3(0), 3.0f);
    
    this->functions.push_back(new SphereDistanceFunction(glm::vec3(0), 1.0f));
    this->functions.push_back(new BoxDistanceFunction(glm::vec3(0), glm::vec3(1.0f)));
    this->functions.push_back(new CapsuleDistanceFunction(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), 1.0f));
    this->currentFunction = this->functions[0];
    this->detail = 1.0f;
    this->mode = BrushMode::ADD;
    this->brushIndex = 0;
}



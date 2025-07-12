#include "tools.hpp"

BrushContext::BrushContext(Camera *camera, Scene &scene) : camera(camera) , scene(scene) {
    this->simplifier = new Simplifier(0.99f, 0.01f, true);
    this->boundingVolume = BoundingSphere(glm::vec3(0), 3.0f);
    
    this->functions.push_back(new SphereDistanceFunction(glm::vec3(0), 1.0f));
    this->functions.push_back(new BoxDistanceFunction(glm::vec3(0), glm::vec3(1.0f)));
    this->functions.push_back(new CapsuleDistanceFunction(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), 1.0f));
    this->functions.push_back(new OctahedronDistanceFunction(glm::vec3(0), 1.0f));
    this->currentFunction = this->functions[0];
    this->detail = 1.0f;
    this->mode = BrushMode::ADD;
    this->brushIndex = 0;
}

WrappedSignedDistanceFunction * BrushContext::getWrapped() {
    if(currentFunction == functions[0]) {
        SphereDistanceFunction * function = (SphereDistanceFunction*) currentFunction;
        return new WrappedSphere(function, detail);
    }
    else if(currentFunction == functions[1]) {
        BoxDistanceFunction * function = (BoxDistanceFunction*) currentFunction;
        return new WrappedBox(function, detail);
    }
    else if(currentFunction == functions[2]) {
        CapsuleDistanceFunction * function = (CapsuleDistanceFunction*) currentFunction;
        return new WrappedCapsule(function, detail);
    }
    else if(currentFunction == functions[3]) {
        OctahedronDistanceFunction * function = (OctahedronDistanceFunction*) currentFunction;
        return new WrappedOctahedron(function, detail);
    }
    return NULL;
}


void BrushContext::apply(Octree &space, OctreeChangeHandler &handler, bool preview) {
    WrappedSignedDistanceFunction * wrapped = getWrapped();
    if(wrapped) {
        glm::quat quaternion = glm::angleAxis(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f)) * // Yaw (Y-axis)
                                glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f)) * // Pitch (X-axis)
                                glm::angleAxis(glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));  // Roll (Z-axis)

        if(preview  || mode == BrushMode::ADD) {
            space.add(*wrapped, quaternion, SimpleBrush(brushIndex), detail, *simplifier, handler);
        } else {
            space.del(*wrapped, quaternion, SimpleBrush(brushIndex), detail, *simplifier, handler);
        }
        delete wrapped;
    }
}

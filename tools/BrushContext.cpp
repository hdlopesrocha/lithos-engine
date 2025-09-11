#include "tools.hpp"

BrushContext::BrushContext(Settings * settings, Camera * camera) : settings(settings), camera(camera), model(glm::vec3(1.0f),glm::vec3(0.0f), 0.0f, 0.0f, 0.0f) {
    this->simplifier = new Simplifier(0.99f, 0.1f, true);
    
    this->functions.push_back(new SphereDistanceFunction());
    this->functions.push_back(new BoxDistanceFunction());
    this->functions.push_back(new CapsuleDistanceFunction(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), 1.0f));
    this->functions.push_back(new OctahedronDistanceFunction());
    this->functions.push_back(new PyramidDistanceFunction());
    this->functions.push_back(new TorusDistanceFunction(glm::vec2(0.25f, 0.25f)));
    this->currentFunction = this->functions[0];
    this->detail = 1.0f;
    this->mode = BrushMode::ADD;
    this->brushIndex = 0;
    this->currentTab = Tab::PAGE_ROTATION;

}

WrappedSignedDistanceFunction * BrushContext::getWrapped() {
    if(currentFunction == functions[0]) {
        SphereDistanceFunction * function = (SphereDistanceFunction*) currentFunction;
        return new WrappedSphere(function, detail, model);
    }
    else if(currentFunction == functions[1]) {
        BoxDistanceFunction * function = (BoxDistanceFunction*) currentFunction;
        return new WrappedBox(function, detail, model);
    }
    else if(currentFunction == functions[2]) {
        CapsuleDistanceFunction * function = (CapsuleDistanceFunction*) currentFunction;
        return new WrappedCapsule(function, detail, model);
    }
    else if(currentFunction == functions[3]) {
        OctahedronDistanceFunction * function = (OctahedronDistanceFunction*) currentFunction;
        return new WrappedOctahedron(function, detail, model);
    }
    else if(currentFunction == functions[4]) {
        PyramidDistanceFunction * function = (PyramidDistanceFunction*) currentFunction;
        return new WrappedPyramid(function, detail, model);
    }
    else if(currentFunction == functions[5]) {
        TorusDistanceFunction * function = (TorusDistanceFunction*) currentFunction;
        return new WrappedTorus(function, detail, model);
    }
    return NULL;
}


void BrushContext::apply(Octree &space, OctreeChangeHandler * handler, bool preview) {
    WrappedSignedDistanceFunction * wrapped = getWrapped();
    if(wrapped) {
        float safeDetail = glm::ceil(wrapped->getLength() * settings->safetyDetailRatio);
        if(detail < safeDetail) {
            detail = safeDetail;
            std::cout << "BrushContext::apply: detail increased to " << std::to_string(detail) << std::endl;
        }
        if(preview  || mode == BrushMode::ADD) {
            space.add(wrapped, this->model, SimpleBrush(brushIndex), detail, *simplifier, handler);
        } else {
            space.del(wrapped, this->model, SimpleBrush(brushIndex), detail, *simplifier, handler);
        }
        delete wrapped;
    }
}

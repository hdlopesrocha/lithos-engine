#include "tools.hpp"

BrushContext::BrushContext(Settings * settings, Camera * camera) : settings(settings), camera(camera), model(glm::vec3(1.0f),glm::vec3(0.0f), 0.0f, 0.0f, 0.0f) {
    this->simplifier = new Simplifier(0.99f, 0.1f, true);
    
    this->functions.push_back(new WrappedSphere(new SphereDistanceFunction()));
    this->functions.push_back(new WrappedBox(new BoxDistanceFunction()));
    this->functions.push_back(new WrappedCapsule(new CapsuleDistanceFunction(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), 1.0f)));
    this->functions.push_back(new WrappedOctahedron(new OctahedronDistanceFunction()));
    this->functions.push_back(new WrappedPyramid(new PyramidDistanceFunction()));
    this->functions.push_back(new WrappedTorus(new TorusDistanceFunction(glm::vec2(0.75f, 0.25f))));
    this->functions.push_back(new WrappedCone(new ConeDistanceFunction()));

    this->currentFunction = this->functions[0];
    this->detail = 1.0f;
    this->mode = BrushMode::ADD;
    this->brushIndex = 0;
    this->currentTab = Tab::PAGE_ROTATION;

}

void BrushContext::apply(Octree &space, OctreeChangeHandler * handler, bool preview) {
    if(currentFunction) {
        float safeDetail = glm::ceil(currentFunction->getLength(this->model, detail) * settings->safetyDetailRatio);
        if(detail < safeDetail) {
            detail = safeDetail;
            std::cout << "BrushContext::apply: detail increased to " << std::to_string(detail) << std::endl;
        }
        if(preview  || mode == BrushMode::ADD) {
            space.add(currentFunction, this->model, SimpleBrush(brushIndex), detail, *simplifier, handler);
        } else {
            space.del(currentFunction, this->model, SimpleBrush(brushIndex), detail, *simplifier, handler);
        }
    }
}

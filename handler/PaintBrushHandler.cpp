#include "handler.hpp"

PaintBrushHandler::PaintBrushHandler(Brush3d &brush3d, Scene &scene) : 
    brush3d(brush3d), 
    octree(*scene.solidSpace),
    simplifier(0.99, 0.1, true)
     {
}

void PaintBrushHandler::handle(Event event) {
    std::unique_ptr<ContainmentHandler> handler;
    std::unique_ptr<SignedDistanceFunction> function;
/*
    if(brush3d.shape == BrushShape::BOX) {
        BoundingBox box = BoundingBox(brush3d.position- brush3d.scale*0.5f, brush3d.position + brush3d.scale*0.5f);
        glm::vec3 shift = glm::vec3(brush3d.detail*2);
        BoundingBox box2 = BoundingBox(box.getMin() - shift, box.getMax() + shift);
        handler = std::make_unique<BoxContainmentHandler>(box2);
        function = std::make_unique<BoxDistanceFunction>(box);
    } else {
        BoundingSphere sphere = BoundingSphere(brush3d.position, brush3d.scale.x);
        BoundingSphere sphere2 = BoundingSphere(sphere.center, sphere.radius + brush3d.detail*2);
        handler = std::make_unique<SphereContainmentHandler>(sphere2);
        function = std::make_unique<SphereDistanceFunction>(sphere);
    }

    if(brush3d.mode == BrushMode::ADD) {
        octree.add(*handler, *function, SimpleBrush(brush3d.index), brush3d.detail, simplifier);
    } else if(brush3d.mode == BrushMode::REMOVE) {
        octree.del(*handler, *function, SimpleBrush(brush3d.index), brush3d.detail, simplifier);
    }
*/
}


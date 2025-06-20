#include "command.hpp"

PaintBrushCommand::PaintBrushCommand(Brush3d &brush3d, Scene &scene) : 
    brush3d(brush3d), 
    octree(*scene.solidSpace), 
    dirtyHandler(scene),
    simplifier(0.99, 0.1, true)
     {
}

void PaintBrushCommand::execute(const float &value) {
    std::unique_ptr<ContainmentHandler> handler;
    std::unique_ptr<SignedDistanceFunction> function;

    if(brush3d.shape == BrushShape::BOX) {
        BoundingBox box = BoundingBox(brush3d.position- brush3d.scale*0.5f, brush3d.position + brush3d.scale*0.5f);
        handler = std::make_unique<BoxContainmentHandler>(box, SimpleBrush(brush3d.index));
        function = std::make_unique<BoxDistanceFunction>(box);
    } else {
        BoundingSphere sphere = BoundingSphere(brush3d.position, brush3d.scale.x);
        handler = std::make_unique<SphereContainmentHandler>(sphere, SimpleBrush(brush3d.index));
        function = std::make_unique<SphereDistanceFunction>(sphere);
    }

    if(brush3d.mode == BrushMode::ADD) {
        octree.add(*handler, *function, dirtyHandler, brush3d.detail, simplifier);
    } else if(brush3d.mode == BrushMode::REMOVE) {
        octree.del(*handler, *function, dirtyHandler, brush3d.detail, simplifier);
    }

}


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

    if(brush3d.shape == BrushShape::BOX) {
        handler = std::make_unique<BoxContainmentHandler>(
            BoundingBox(brush3d.position- brush3d.scale*0.5f, brush3d.position + brush3d.scale*0.5f),
            SimpleBrush(brush3d.index)
        );
    } else {
        handler = std::make_unique<SphereContainmentHandler>(
            BoundingSphere(brush3d.position, brush3d.scale.x),
            SimpleBrush(brush3d.index)
        );
    }

    if(brush3d.mode == BrushMode::ADD) {
        octree.add(*handler, dirtyHandler, brush3d.detail, simplifier);
    } else if(brush3d.mode == BrushMode::REMOVE) {
        octree.del(*handler, dirtyHandler, brush3d.detail, simplifier);
    }

}


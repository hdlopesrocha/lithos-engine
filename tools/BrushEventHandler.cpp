#include "tools.hpp"


template<typename T> BrushEventHandler<T>::BrushEventHandler(BrushContext &context, Scene &scene) : context(context), scene(scene) {
}

template<typename T> void BrushEventHandler<T>::handle(T * event) {
    bool changed = false;

    if(event->getType() == EVENT_NEXT_TAB) {
        int tab = static_cast<int>(context.currentTab) + 1;
        int count = static_cast<int>(Tab::COUNT);
        context.currentTab = static_cast<Tab>(Math::mod(tab, count));
    } 
    if(event->getType() == EVENT_PREVIOUS_TAB) {
        int tab = static_cast<int>(context.currentTab) - 1;
        int count = static_cast<int>(Tab::COUNT);
        context.currentTab = static_cast<Tab>(Math::mod(tab, count));
    } 


    switch (context.currentTab)
    {
    case PAGE_ROTATION:
        if(event->getType() == EVENT_VECTOR_3D_0) {
            Axis3dEvent * e = (Axis3dEvent*) event;
            TranslateHandler(context.camera, &(context.model.translate)).handle(e);
            changed = true;
        }
        if(event->getType() == EVENT_VECTOR_3D_1) {
            Axis3dEvent * e = (Axis3dEvent*) event;
            RotateHandler(context.camera, &(context.model.quaternion)).handle(e);
            changed = true;
        } 
        break;
    case PAGE_SCALE:
        if(event->getType() == EVENT_VECTOR_3D_0) {
            Axis3dEvent * e = (Axis3dEvent*) event;
            TranslateHandler(context.camera, &(context.model.translate)).handle(e);
            changed = true;
        }
        if(event->getType() == EVENT_FLOAT_1_X) {
            FloatEvent * e = (FloatEvent*) event;
            ScaleHandler(&(context.model.scale.x)).handle(e);
            changed = true;
        } if(event->getType() == EVENT_FLOAT_1_Y) {
            FloatEvent * e = (FloatEvent*) event;
            ScaleHandler(&(context.model.scale.y)).handle(e);
            changed = true;
        } if(event->getType() == EVENT_FLOAT_1_Z) {
            FloatEvent * e = (FloatEvent*) event;
            ScaleHandler(&(context.model.scale.z)).handle(e);
            changed = true;
        }
        break;
    case PAGE_TRANSLATE:
        if(event->getType() == EVENT_VECTOR_3D_0) {
            Axis3dEvent * e = (Axis3dEvent*) event;
            TranslateHandler(context.camera, &(context.model.translate)).handle(e);
            changed = true;
        }
        break;        
    case PAGE_SDF0:
        if(context.currentFunction == context.functions[0]) {
            SphereDistanceFunction * function = (SphereDistanceFunction*) context.currentFunction;
            if(event->getType() == EVENT_VECTOR_3D_0) {
                Axis3dEvent * e = (Axis3dEvent*) event;
                TranslateHandler(context.camera, &(function->center)).handle(e);
                changed = true;
            }
            if(event->getType() == EVENT_FLOAT_1_X) {
                FloatEvent * e = (FloatEvent*) event;
                ScaleHandler(&(function->radius)).handle(e);
                changed = true;
            }
        }

        if(context.currentFunction == context.functions[1]) {
            BoxDistanceFunction * function = (BoxDistanceFunction*) context.currentFunction;
            if(event->getType() == EVENT_VECTOR_3D_0) {
                Axis3dEvent * e = (Axis3dEvent*) event;
                TranslateHandler(context.camera, &(function->center)).handle(e);
                changed = true;
            } 
            if(event->getType() == EVENT_FLOAT_1_X) {
                FloatEvent * e = (FloatEvent*) event;
                ScaleHandler(&(function->length.x)).handle(e);
                changed = true;
            }
            if(event->getType() == EVENT_FLOAT_1_Y) {
                FloatEvent * e = (FloatEvent*) event;
                ScaleHandler(&(function->length.y)).handle(e);
                changed = true;
            }
            if(event->getType() == EVENT_FLOAT_1_Z) {
                FloatEvent * e = (FloatEvent*) event;
                ScaleHandler(&(function->length.z)).handle(e);
                changed = true;
            }
        }

        if(context.currentFunction == context.functions[2]) {
            CapsuleDistanceFunction * function = (CapsuleDistanceFunction*) context.currentFunction;
            if(event->getType() == EVENT_VECTOR_3D_0) {
                Axis3dEvent * e = (Axis3dEvent*) event;
                TranslateHandler(context.camera, &(function->a)).handle(e);
                changed = true;
            } 
            if(event->getType() == EVENT_VECTOR_3D_2) {
                Axis3dEvent * e = (Axis3dEvent*) event;
                TranslateHandler(context.camera, &(function->b)).handle(e);
                changed = true;
            }
            if(event->getType() == EVENT_FLOAT_1_X) {
                FloatEvent * e = (FloatEvent*) event;
                ScaleHandler(&(function->radius)).handle(e);
                changed = true;
            }
        }

        if(context.currentFunction == context.functions[3]) {
            OctahedronDistanceFunction * function = (OctahedronDistanceFunction*) context.currentFunction;
            if(event->getType() == EVENT_VECTOR_3D_0) {
                Axis3dEvent * e = (Axis3dEvent*) event;
                TranslateHandler(context.camera, &(function->center)).handle(e);
                changed = true;
            }
            if(event->getType() == EVENT_FLOAT_1_X) {
                FloatEvent * e = (FloatEvent*) event;
                ScaleHandler(&(function->radius)).handle(e);
                changed = true;
            }
        }

        if(context.currentFunction == context.functions[4]) {
            PyramidDistanceFunction * function = (PyramidDistanceFunction*) context.currentFunction;
            if(event->getType() == EVENT_VECTOR_3D_0) {
                Axis3dEvent * e = (Axis3dEvent*) event;
                TranslateHandler(context.camera, &(function->base)).handle(e);
                changed = true;
            }
        }
        break;
    
    default:
        break;
    }
    
    if(event->getType() == EVENT_PAINT_BRUSH) {
        std::cout << "EVENT_PAINT_BRUSH" << std::endl;
        context.apply(scene.solidSpace, scene.solidSpaceChangeHandler, false);
    }

    if(event->getType() == EVENT_NEXT_TAB) {
        std::cout << "EVENT_NEXT_TAB:" << std::to_string(context.currentTab) << std::endl;
    }

    if(event->getType() == EVENT_PREVIOUS_TAB) {
        std::cout << "EVENT_PREVIOUS_TAB:" << std::to_string(context.currentTab) << std::endl;
    }

    if(event->getType() == EVENT_BRUSH_CHANGED) {
        std::cout << "EVENT_BRUSH_CHANGED" << std::endl;
        changed = true;
    }

    if(changed) {
        Octree * space = &scene.brushSpace;
        space->root->clear(&space->allocator, *space, scene.brushSpaceChangeHandler);
        context.apply(*space, scene.brushSpaceChangeHandler, true);
    }
};
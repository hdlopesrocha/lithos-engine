#include "tools.hpp"


template<typename T> BrushEventHandler<T>::BrushEventHandler(BrushContext &context, Octree &brushSpace) : context(context), brushSpace(brushSpace) {

}

template<typename T> void BrushEventHandler<T>::handle(T * event) {
    bool changed = false;

    if(context.currentFunction == context.functions[0]) {
        SphereDistanceFunction * function = (SphereDistanceFunction*) context.currentFunction;
        if(event->getType() == EVENT_VECTOR_3D_0) {
            Axis3dEvent * e = (Axis3dEvent*) event;
            TranslateHandler(context.camera, &(function->center)).handle(e);
            changed = true;
            std::cout << "EVENT_VECTOR_3D_0 = " << std::to_string(e->axis.x)<< ":"<< std::to_string(e->axis.y)<< ":"<<  std::to_string(e->axis.z) << std::endl;
        }
        if(event->getType() == EVENT_FLOAT_1_X) {
            FloatEvent * e = (FloatEvent*) event;
            FloatHandler(&(function->radius)).handle(e);
            changed = true;
            std::cout << "EVENT_FLOAT_1_X = " << std::to_string(e->value) << std::endl;
        }
    }

    if(context.currentFunction == context.functions[1]) {
        BoxDistanceFunction * function = (BoxDistanceFunction*) context.currentFunction;
        if(event->getType() == EVENT_VECTOR_3D_0) {
            Axis3dEvent * e = (Axis3dEvent*) event;
            TranslateHandler(context.camera, &(function->center)).handle(e);
            changed = true;
            std::cout << "EVENT_VECTOR_3D_0 = " << std::to_string(e->axis.x)<< ":"<< std::to_string(e->axis.y)<< ":"<<  std::to_string(e->axis.z) << std::endl;
        } 
        if(event->getType() == EVENT_FLOAT_1_X) {
            FloatEvent * e = (FloatEvent*) event;
            ScaleHandler(&(function->length.x)).handle(e);
            changed = true;
            std::cout << "EVENT_FLOAT_1_X = " << std::to_string(e->value)<< std::endl;
        }
        if(event->getType() == EVENT_FLOAT_1_Y) {
            FloatEvent * e = (FloatEvent*) event;
            ScaleHandler(&(function->length.y)).handle(e);
            changed = true;
            std::cout << "EVENT_FLOAT_1_Y = " << std::to_string(e->value)<< std::endl;
        }
        if(event->getType() == EVENT_FLOAT_1_Z) {
            FloatEvent * e = (FloatEvent*) event;
            ScaleHandler(&(function->length.z)).handle(e);
            changed = true;
            std::cout << "EVENT_FLOAT_1_Z = " << std::to_string(e->value)<< std::endl;
        }
    }

    if(context.currentFunction == context.functions[2]) {
        CapsuleDistanceFunction * function = (CapsuleDistanceFunction*) context.currentFunction;
        if(event->getType() == EVENT_VECTOR_3D_0) {
            Axis3dEvent * e = (Axis3dEvent*) event;
            TranslateHandler(context.camera, &(function->a)).handle(e);
            changed = true;
            std::cout << "EVENT_VECTOR_3D_0 = " << std::to_string(e->axis.x)<< ":"<< std::to_string(e->axis.y)<< ":"<<  std::to_string(e->axis.z) << std::endl;
        } 
        if(event->getType() == EVENT_VECTOR_3D_2) {
            Axis3dEvent * e = (Axis3dEvent*) event;
            TranslateHandler(context.camera, &(function->b)).handle(e);
            changed = true;
            std::cout << "EVENT_VECTOR_3D_2 = " << std::to_string(e->axis.x)<< ":"<< std::to_string(e->axis.y)<< ":"<<  std::to_string(e->axis.z) << std::endl;
        }
        if(event->getType() == EVENT_FLOAT_1_X) {
            FloatEvent * e = (FloatEvent*) event;
            FloatHandler(&(function->radius)).handle(e);
            changed = true;
            std::cout << "EVENT_FLOAT_1_X = " << std::to_string(e->value) << std::endl;
        }
    }

    if(changed) {
        brushSpace.root->clear(&brushSpace.allocator, brushSpace);
        context.apply(brushSpace);
    }
};
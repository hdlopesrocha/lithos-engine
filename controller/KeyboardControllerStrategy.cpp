#include "controller.hpp"



KeyboardControllerStrategy::KeyboardControllerStrategy(LithosApplication &app, EventManager &eventManager): app(app), eventManager(eventManager) {
 
}

void KeyboardControllerStrategy::handleInput(float deltaTime) {
    glm::vec3 vector3d0 = glm::vec3(0);
    glm::vec3 vector3d1 = glm::vec3(0);
    glm::vec3 component3d0 = glm::vec3(0);
    glm::vec3 component3d2 = glm::vec3(0);

    if (app.getKeyboardStatus(GLFW_KEY_A) != GLFW_RELEASE) {
        vector3d0.x = -1;
        component3d0.x = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_D) != GLFW_RELEASE) {
        vector3d0.x = 1;
        component3d0.z = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_W) != GLFW_RELEASE) {
        vector3d0.z = 1;
        component3d0.y = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_S) != GLFW_RELEASE) {
        vector3d0.z = -1;
        component3d0.y = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_Q) != GLFW_RELEASE) {
        vector3d0.y = -1;
        component3d0.x = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_E) != GLFW_RELEASE) {
        vector3d0.y = 1;
        component3d0.z = 1;
    }


    if (app.getKeyboardStatus(GLFW_KEY_J) != GLFW_RELEASE) {
        vector3d1.x = -1;
        component3d2.x = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_L) != GLFW_RELEASE) {
        vector3d1.x = 1;
        component3d2.z = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_I) != GLFW_RELEASE) {
        vector3d1.z = 1;
        component3d2.y = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_K) != GLFW_RELEASE) {
        vector3d1.z = -1;
        component3d2.y = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_U) != GLFW_RELEASE) {
        vector3d1.y = -1;
        component3d2.x = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_O) != GLFW_RELEASE) {
        vector3d1.y = 1;
        component3d2.z = 1;
    }

    if (app.getKeyboardStatus(GLFW_KEY_SPACE) != GLFW_RELEASE) {
        eventManager.publish<Event>(Event(EVENT_PAINT_BRUSH));
    }

    if (app.getKeyboardStatus(GLFW_KEY_Q) != GLFW_RELEASE) {
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_0_X, 1.0f));
    }
    if (app.getKeyboardStatus(GLFW_KEY_A) != GLFW_RELEASE) {
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_0_X, -1.0f));
    }
    if (app.getKeyboardStatus(GLFW_KEY_W) != GLFW_RELEASE) {
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_0_Y, 1.0f));
    }
    if (app.getKeyboardStatus(GLFW_KEY_S) != GLFW_RELEASE) {
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_0_Y, -1.0f));
    }
    if (app.getKeyboardStatus(GLFW_KEY_E) != GLFW_RELEASE) {
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_0_Z, 1.0f));
    }
    if (app.getKeyboardStatus(GLFW_KEY_D) != GLFW_RELEASE) {
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_0_Z, -1.0f));
    }

    if (app.getKeyboardStatus(GLFW_KEY_R) != GLFW_RELEASE) {
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_1_X, 1.0f));
    }
    if (app.getKeyboardStatus(GLFW_KEY_F) != GLFW_RELEASE) {
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_1_X, -1.0f));
    }
    if (app.getKeyboardStatus(GLFW_KEY_T) != GLFW_RELEASE) {
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_1_Y, 1.0f));
    }
    if (app.getKeyboardStatus(GLFW_KEY_G) != GLFW_RELEASE) {
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_1_Y, -1.0f));
    }
        if (app.getKeyboardStatus(GLFW_KEY_Y) != GLFW_RELEASE) {
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_1_Z, 1.0f));
    }
    if (app.getKeyboardStatus(GLFW_KEY_H) != GLFW_RELEASE) {
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_1_Z, -1.0f));
    }

    if (app.getKeyboardStatus(GLFW_KEY_U) != GLFW_RELEASE) {
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_2_X, 1.0f));
    }
    if (app.getKeyboardStatus(GLFW_KEY_J) != GLFW_RELEASE) {
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_2_X, -1.0f));
    }
    if (app.getKeyboardStatus(GLFW_KEY_I) != GLFW_RELEASE) {
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_2_Y, 1.0f));
    }
    if (app.getKeyboardStatus(GLFW_KEY_K) != GLFW_RELEASE) {
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_2_Y, -1.0f));
    }
    if (app.getKeyboardStatus(GLFW_KEY_O) != GLFW_RELEASE) {
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_2_Z, 1.0f));
    }
    if (app.getKeyboardStatus(GLFW_KEY_L) != GLFW_RELEASE) {
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_2_Z, -1.0f));
    }


    float threshold = 0.2;
    vector3d0 = applyDeadzone(vector3d0, threshold);
    vector3d1 = applyDeadzone(vector3d1, threshold);
    component3d0 = applyDeadzone(component3d0, threshold);
    component3d2 = applyDeadzone(component3d2, threshold);
    
    if(isAboveDeadzone(vector3d0, threshold)) {
        eventManager.publish<Axis3dEvent>(Axis3dEvent(EVENT_VECTOR_3D_0, vector3d0, deltaTime));
    }

    if(isAboveDeadzone(vector3d1, threshold)) {
        eventManager.publish<Axis3dEvent>(Axis3dEvent(EVENT_VECTOR_3D_1, vector3d1, deltaTime));
    }

    if(isAboveDeadzone(component3d0, threshold)) {
        eventManager.publish<Axis3dEvent>(Axis3dEvent(EVENT_COMPONENT_3D_0, component3d0, deltaTime));
    }

    if(isAboveDeadzone(component3d2, threshold)) {
        eventManager.publish<Axis3dEvent>(Axis3dEvent(EVENT_COMPONENT_3D_2, component3d2, deltaTime));
    }

    if (app.getKeyboardStatus(GLFW_KEY_ESCAPE) != GLFW_RELEASE) {
        eventManager.publish<Event>(Event(EVENT_CLOSE_WINDOW));
    }

    if (app.getKeyboardStatus(GLFW_KEY_X) != GLFW_RELEASE) {
        if(!keyWasPressed[GLFW_KEY_X]) { 
            eventManager.publish<BinaryEvent>(BinaryEvent(EVENT_NEXT_PAGE, true));
        }
        keyWasPressed[GLFW_KEY_X] = true;
    } else {
        keyWasPressed[GLFW_KEY_X] = false;
    }

    if (app.getKeyboardStatus(GLFW_KEY_Z) != GLFW_RELEASE) {
        if(!keyWasPressed[GLFW_KEY_Z]) { 
            eventManager.publish<BinaryEvent>(BinaryEvent(EVENT_PREVIOUS_PAGE, false));
        }
        keyWasPressed[GLFW_KEY_Z] = true;
    } else {
        keyWasPressed[GLFW_KEY_Z] = false;
    }

}

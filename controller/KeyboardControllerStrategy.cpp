#include "controller.hpp"



KeyboardControllerStrategy::KeyboardControllerStrategy(LithosApplication &app, EventManager &eventManager): app(app), eventManager(eventManager) {
 
}

void KeyboardControllerStrategy::handleInput(float deltaTime) {
    glm::vec3 vector3d0 = glm::vec3(0);
    glm::vec3 vector3d1 = glm::vec3(0);

    if (app.getKeyboardStatus(GLFW_KEY_A) != GLFW_RELEASE) {
        vector3d0.x = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_D) != GLFW_RELEASE) {
        vector3d0.x = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_W) != GLFW_RELEASE) {
        vector3d0.z = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_S) != GLFW_RELEASE) {
        vector3d0.z = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_Q) != GLFW_RELEASE) {
        vector3d0.y = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_E) != GLFW_RELEASE) {
        vector3d0.y = 1;
    }


    if (app.getKeyboardStatus(GLFW_KEY_J) != GLFW_RELEASE) {
        vector3d1.x = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_L) != GLFW_RELEASE) {
        vector3d1.x = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_I) != GLFW_RELEASE) {
        vector3d1.z = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_K) != GLFW_RELEASE) {
        vector3d1.z = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_U) != GLFW_RELEASE) {
        vector3d1.y = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_O) != GLFW_RELEASE) {
        vector3d1.y = 1;
    }

    if (app.getKeyboardStatus(GLFW_KEY_SPACE) != GLFW_RELEASE) {
        eventManager.publish<float>(EVENT_PAINT_BRUSH, 1.0f);
    }

    float threshold = 0.2;
    vector3d0 = applyDeadzone(vector3d0, threshold);
    
    if(isAboveDeadzone(vector3d0, threshold)) {
        eventManager.publish<Axis3dEvent>(EVENT_VECTOR_3D_0, Axis3dEvent(vector3d0, deltaTime));
    }

    if(isAboveDeadzone(vector3d1, threshold)) {
        eventManager.publish<Axis3dEvent>(EVENT_VECTOR_3D_1, Axis3dEvent(vector3d1, deltaTime));
    }

    if (app.getKeyboardStatus(GLFW_KEY_ESCAPE) != GLFW_RELEASE) {
        eventManager.publish<float>(EVENT_CLOSE_WINDOW, 1.0f);
    }

    if (app.getKeyboardStatus(GLFW_KEY_2) != GLFW_RELEASE) {
        if(!keyWasPressed[GLFW_KEY_2]) { 
            eventManager.publish<float>(EVENT_NEXT_PAGE, 1.0f);
        }
        keyWasPressed[GLFW_KEY_2] = true;
    } else {
        keyWasPressed[GLFW_KEY_2] = false;
    }

    if (app.getKeyboardStatus(GLFW_KEY_1) != GLFW_RELEASE) {
        if(!keyWasPressed[GLFW_KEY_1]) { 
            eventManager.publish<float>(EVENT_PREVIOUS_PAGE, 1.0f);
        }
        keyWasPressed[GLFW_KEY_1] = true;
    } else {
        keyWasPressed[GLFW_KEY_1] = false;
    }

}

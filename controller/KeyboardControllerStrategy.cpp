#include "controller.hpp"

float KeyboardControllerStrategy::tick(int key, float deltaTime) {
    if(deltaTime!=0.0) {
        keyDuration[key] += deltaTime;
    } else {
        keyDuration[key] = 0.0f;
    }
    return keyDuration[key];
}



KeyboardControllerStrategy::KeyboardControllerStrategy(LithosApplication &app, EventManager &eventManager): app(app), eventManager(eventManager) {
 
}

void KeyboardControllerStrategy::handleInput(float deltaTime) {
    glm::vec3 vector3d0 = glm::vec3(0);
    glm::vec3 vector3d1 = glm::vec3(0);
    glm::vec3 vector3d2 = glm::vec3(0);
    glm::vec3 float3d0 = glm::vec3(0);
    glm::vec3 float3d1 = glm::vec3(0);
    glm::vec3 float3d2 = glm::vec3(0);

    if (app.getKeyboardStatus(GLFW_KEY_A) != GLFW_RELEASE) {
        vector3d0.x = -1;
        float3d0.x = -1;

    }
    if (app.getKeyboardStatus(GLFW_KEY_D) != GLFW_RELEASE) {
        vector3d0.x = 1;
        float3d0.z = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_W) != GLFW_RELEASE) {
        vector3d0.z = 1;
        float3d0.y = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_S) != GLFW_RELEASE) {
        vector3d0.z = -1;
        float3d0.y = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_Q) != GLFW_RELEASE) {
        vector3d0.y = -1;
        float3d0.x = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_E) != GLFW_RELEASE) {
        vector3d0.y = 1;
        float3d0.z = 1;
    }

    if (app.getKeyboardStatus(GLFW_KEY_F) != GLFW_RELEASE) {
        vector3d1.x = -1;
        float3d1.x = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_H) != GLFW_RELEASE) {
        vector3d1.x = 1;
        float3d1.z = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_T) != GLFW_RELEASE) {
        vector3d1.z = 1;
        float3d1.y = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_G) != GLFW_RELEASE) {
        vector3d1.z = -1;
        float3d1.y = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_R) != GLFW_RELEASE) {
        vector3d1.y = -1;
        float3d1.x = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_Y) != GLFW_RELEASE) {
        vector3d1.y = 1;
        float3d1.z = 1;
    }

    if (app.getKeyboardStatus(GLFW_KEY_J) != GLFW_RELEASE) {
        vector3d2.x = -1;
        float3d2.x = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_L) != GLFW_RELEASE) {
        vector3d2.x = 1;
        float3d2.z = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_I) != GLFW_RELEASE) {
        vector3d2.z = 1;
        float3d2.y = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_K) != GLFW_RELEASE) {
        vector3d2.z = -1;
        float3d2.y = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_U) != GLFW_RELEASE) {
        vector3d2.y = -1;
        float3d2.x = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_O) != GLFW_RELEASE) {
        vector3d2.y = 1;
        float3d2.z = 1;
    }

    if (app.getKeyboardStatus(GLFW_KEY_SPACE) != GLFW_RELEASE) {
        eventManager.publish<Event>(Event(EVENT_PAINT_BRUSH));
    }

    float threshold = 0.2;
    vector3d0 = applyDeadzone(vector3d0, threshold);
    vector3d1 = applyDeadzone(vector3d1, threshold);
    vector3d2 = applyDeadzone(vector3d2, threshold);

    float3d0 = applyDeadzone(float3d0, threshold);
    float3d1 = applyDeadzone(float3d1, threshold);
    float3d2 = applyDeadzone(float3d2, threshold);

    if(float3d0.x){
        float duration = tick(EVENT_FLOAT_0_X, deltaTime);
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_0_X, float3d0.x, deltaTime, duration));
    } else {
        tick(EVENT_FLOAT_0_X, 0.0f);
    }
    if(float3d0.y){
        float duration = tick(EVENT_FLOAT_0_Y, deltaTime);
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_0_Y, float3d0.y, deltaTime, duration));
    } else {
        tick(EVENT_FLOAT_0_Y, 0.0f);
    }
    if(float3d0.z){
        float duration = tick(EVENT_FLOAT_0_Z, deltaTime);
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_0_Z, float3d0.z, deltaTime, duration));
    } else {
        tick(EVENT_FLOAT_0_Z, 0.0f);
    }
    if(isAboveDeadzone(vector3d0, threshold)) {
        float duration = tick(EVENT_VECTOR_3D_0, deltaTime);
        eventManager.publish<Axis3dEvent>(Axis3dEvent(EVENT_VECTOR_3D_0, vector3d0, deltaTime, duration));
    } else {
        tick(EVENT_VECTOR_3D_0, 0.0f);
    }

    if(float3d1.x){
        float duration = tick(EVENT_FLOAT_1_X, deltaTime);
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_1_X, float3d1.x, deltaTime, duration));
    } else {
        tick(EVENT_FLOAT_1_X, 0.0f);
    }
    if(float3d1.y){
        float duration = tick(EVENT_FLOAT_1_Y, deltaTime);
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_1_Y, float3d1.y, deltaTime, duration));
    } else {
        tick(EVENT_FLOAT_1_Y, 0.0f);
    }
    if(float3d1.z){
        float duration = tick(EVENT_FLOAT_1_Z, deltaTime);
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_1_Z, float3d1.z, deltaTime, duration));
    } else {
        tick(EVENT_FLOAT_1_Z, 0.0f);
    }
    if(isAboveDeadzone(vector3d1, threshold)) {
        float duration = tick(EVENT_VECTOR_3D_1, deltaTime);
        eventManager.publish<Axis3dEvent>(Axis3dEvent(EVENT_VECTOR_3D_1, vector3d1, deltaTime, duration));
    } else {
        tick(EVENT_VECTOR_3D_1, 0.0f);
    }

  
    if(float3d2.x){
        float duration = tick(EVENT_FLOAT_2_X, deltaTime);
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_2_X, float3d2.x, deltaTime, duration));
    } else {
        tick(EVENT_FLOAT_2_X, 0.0f);
    }
    if(float3d2.y){
        float duration = tick(EVENT_FLOAT_2_Y, deltaTime);
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_2_Y, float3d2.y, deltaTime, duration));
    } else {
        tick(EVENT_FLOAT_2_Y, 0.0f);
    }
    if(float3d2.z){
        float duration = tick(EVENT_FLOAT_2_Z, deltaTime);
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_2_Z, float3d2.z, deltaTime, duration));
    } else {
        tick(EVENT_FLOAT_0_X, 0.0f);
    }
    if(isAboveDeadzone(vector3d2, threshold)) {
        float duration = tick(EVENT_VECTOR_3D_2, deltaTime);
        eventManager.publish<Axis3dEvent>(Axis3dEvent(EVENT_VECTOR_3D_2, vector3d2, deltaTime, duration));
    } else {
        tick(EVENT_VECTOR_3D_2, 0.0f);
    }


    if (app.getKeyboardStatus(GLFW_KEY_ESCAPE) != GLFW_RELEASE) {
        eventManager.publish<Event>(Event(EVENT_CLOSE_WINDOW));
    }

    if (app.getKeyboardStatus(GLFW_KEY_X) != GLFW_RELEASE) {
        if(!keyWasPressed[GLFW_KEY_X]) { 
            eventManager.publish<Event>(Event(EVENT_NEXT_PAGE));
        }
        keyWasPressed[GLFW_KEY_X] = true;
    } else {
        keyWasPressed[GLFW_KEY_X] = false;
    }

    if (app.getKeyboardStatus(GLFW_KEY_Z) != GLFW_RELEASE) {
        if(!keyWasPressed[GLFW_KEY_Z]) { 
            eventManager.publish<Event>(Event(EVENT_PREVIOUS_PAGE));
        }
        keyWasPressed[GLFW_KEY_Z] = true;
    } else {
        keyWasPressed[GLFW_KEY_Z] = false;
    }

}

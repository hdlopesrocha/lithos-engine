#include "controller.hpp"

float GamepadControllerStrategy::tick(int key, float deltaTime) {
    if(deltaTime!=0.0) {
        keyWasPressed[key] += deltaTime;
    } else {
        keyWasPressed[key] = 0.0f;
    }
    return keyWasPressed[key];
}


GamepadControllerStrategy::GamepadControllerStrategy(EventManager &eventManager) : eventManager(eventManager) {

}

void GamepadControllerStrategy::handleInput(float deltaTime) {
	GLFWgamepadstate state;
    glm::vec3 vector3d0 = glm::vec3(0);
    glm::vec3 vector3d1 = glm::vec3(0);
    glm::vec3 vector3d2 = glm::vec3(0);
    glm::vec3 float3d0 = glm::vec3(0);
    glm::vec3 float3d1 = glm::vec3(0);
    glm::vec3 float3d2 = glm::vec3(0);

    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
        vector3d0.x = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
        vector3d0.y = -state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
        vector3d0.z = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] - state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
        float3d0.x = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
        float3d0.y = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
        float3d0.z = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] - state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];

        vector3d2.x = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
        vector3d2.z = -state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
        vector3d2.y = (state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS ? 1 : 0) - (state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS ? 1 : 0);	
        float3d2.x = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
        float3d2.y = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
        float3d2.z = (state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS ? 1 : 0) - (state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS ? 1 : 0);	
        
        if(state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] == GLFW_PRESS) {
            tick(EVENT_NEXT_PAGE, deltaTime);
            if(!keyWasPressed[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT]) { 
               eventManager.publish<BinaryEvent>(
                    BinaryEvent(EVENT_NEXT_PAGE, true));
            }
        }else {
            tick(EVENT_NEXT_PAGE, 0.0f);
        }
     
        if(state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS) {
            tick(EVENT_PREVIOUS_PAGE, deltaTime);
            if(!keyWasPressed[GLFW_GAMEPAD_BUTTON_DPAD_LEFT]) { 
                eventManager.publish<BinaryEvent>(
                    BinaryEvent(EVENT_PREVIOUS_PAGE, false));
            }
        }else {
            tick(EVENT_PREVIOUS_PAGE, 0.0f);
        }
     

        if(state.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS) {
            tick(EVENT_PAINT_BRUSH, deltaTime);
            eventManager.publish<Event>(Event(EVENT_PAINT_BRUSH));
        } else {
            tick(EVENT_PAINT_BRUSH, 0.0f);
        }
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
}

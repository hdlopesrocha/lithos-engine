#include "controller.hpp"


GamepadControllerStrategy::GamepadControllerStrategy(EventManager &eventManager) : eventManager(eventManager) {

}

void GamepadControllerStrategy::handleInput(float deltaTime) {
	GLFWgamepadstate state;
    glm::vec3 vector3d0 = glm::vec3(0);
    glm::vec3 vector3d1 = glm::vec3(0);
    glm::vec3 vector3d2 = glm::vec3(0);

    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
        vector3d0.x = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
        vector3d0.y = -state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
        vector3d0.z = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] - state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
       
        vector3d2.x = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
        vector3d2.z = -state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
        vector3d2.y = (state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS ? 1 : 0) - (state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS ? 1 : 0);	
    
        if(state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] == GLFW_PRESS) {
            if(!keyWasPressed[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT]) { 
                eventManager.publish<BinaryEvent>(
                    BinaryEvent(EVENT_NEXT_PAGE, true));
            }
            keyWasPressed[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] = true;
        } else {
            keyWasPressed[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] = false;
        }

        if(state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS) {
            if(!keyWasPressed[GLFW_GAMEPAD_BUTTON_DPAD_LEFT]) { 
                eventManager.publish<BinaryEvent>(BinaryEvent(EVENT_PREVIOUS_PAGE, false));
            }
            keyWasPressed[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] = true;
        } else {
            keyWasPressed[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] = false;
        }
        
        if(state.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS) {
            eventManager.publish<Event>(Event(EVENT_PAINT_BRUSH));
        }
    }

    float threshold = 0.2;
    vector3d0 = applyDeadzone(vector3d0, threshold);
    vector3d1 = applyDeadzone(vector3d1, threshold);
    vector3d2 = applyDeadzone(vector3d2, threshold);

    if(isAboveDeadzone(vector3d0, threshold)) {
        eventManager.publish<Axis3dEvent>(Axis3dEvent(EVENT_VECTOR_3D_0, vector3d0, deltaTime));
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_0_X, vector3d0.x));
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_0_Y, vector3d0.y));
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_0_Z, vector3d0.z));
    }

    if(isAboveDeadzone(vector3d1, threshold)) {
        eventManager.publish<Axis3dEvent>(Axis3dEvent(EVENT_VECTOR_3D_1, vector3d1, deltaTime));
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_1_X, vector3d1.x));
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_1_Y, vector3d1.y));
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_1_Z, vector3d1.z));
    }

    if(isAboveDeadzone(vector3d2, threshold)) {
        eventManager.publish<Axis3dEvent>(Axis3dEvent(EVENT_VECTOR_3D_2, vector3d2, deltaTime));
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_2_X, vector3d2.x));
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_2_Y, vector3d2.y));
        eventManager.publish<FloatEvent>(FloatEvent(EVENT_FLOAT_2_Z, vector3d2.z));
    }

}

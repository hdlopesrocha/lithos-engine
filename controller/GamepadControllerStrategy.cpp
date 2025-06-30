#include "controller.hpp"


GamepadControllerStrategy::GamepadControllerStrategy(EventManager &eventManager) : eventManager(eventManager) {

}

void GamepadControllerStrategy::handleInput() {
	GLFWgamepadstate state;
    glm::vec3 vector3d0 = glm::vec3(0);
    glm::vec3 vector3d1 = glm::vec3(0);

    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
        vector3d0.x = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
        vector3d0.y = -state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
        vector3d0.z = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] - state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
       
        vector3d1.y = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
        vector3d1.x = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
        vector3d1.z = (state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS ? 1 : 0) - (state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS ? 1 : 0);	
    
       
        /*
        if(state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS) {
            controllerMode = ControllerMode::CAMERA;
        }
        if(state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS) {
            controllerMode = ControllerMode::BRUSH;
        }
        */
    }

    if(state.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS) {
        eventManager.publish<float>(EVENT_PAINT_BRUSH, 1.0f);
    }

    float threshold = 0.2;
    vector3d0 = applyDeadzone(vector3d0, threshold);
    vector3d1 = applyDeadzone(vector3d1, threshold);
    
    if(isAboveDeadzone(vector3d0, threshold)) {
        eventManager.publish<glm::vec3>(EVENT_VECTOR_3D_0, vector3d0);
    }

    if(isAboveDeadzone(vector3d1, threshold)) {
        eventManager.publish<glm::vec3>(EVENT_VECTOR_3D_1, vector3d1);
    }


}

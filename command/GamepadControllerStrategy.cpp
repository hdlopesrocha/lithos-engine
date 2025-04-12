#include "command.hpp"

GamepadControllerStrategy::GamepadControllerStrategy(Camera &camera, Brush3d &brush3d) : camera(camera), brush3d(brush3d) {
    this->translateCameraCommand = new TranslateCameraCommand(camera);
    this->rotateCameraCommand = new RotateCameraCommand(camera);
}

void GamepadControllerStrategy::handleInput(float deltaTime) {
	GLFWgamepadstate state;
    glm::vec3 translate = glm::vec3(0);
    glm::vec3 rotate = glm::vec3(0);

    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
        //ImGui::Text("Button A: %s", (state.buttons[GLFW_GAMEPAD_BUTTON_A] ? "Pressed" : "Released"));
        rotate.y = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
        rotate.x = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
        rotate.z = (state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS ? 1 : 0) - (state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS ? 1 : 0);	
    
        translate.x = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
        translate.y = -state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
        translate.z = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] - state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
    
    }

    float threshold = 0.2;
    translate = applyDeadzone(translate, threshold);
    rotate = applyDeadzone(rotate, threshold);
    
    if(isAboveDeadzone(translate, threshold)) {
        translateCameraCommand->execute(translate*deltaTime);
    }

    if(isAboveDeadzone(rotate, threshold)) {
        rotateCameraCommand->execute(rotate*deltaTime);
    }
}

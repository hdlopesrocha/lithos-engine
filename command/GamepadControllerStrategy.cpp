#include "command.hpp"


GamepadControllerStrategy::GamepadControllerStrategy(Camera &camera, Brush3d &brush3d, Octree &octree) {
    this->translateCameraCommand = new TranslateCameraCommand(camera);
    this->rotateCameraCommand = new RotateCameraCommand(camera);
    this->translateBrushCommand = new TranslateBrushCommand(brush3d, camera);
    this->paintBrushCommand = new PaintBrushCommand(brush3d, octree);
    this->controllerMode = ControllerMode::CAMERA;
}

void GamepadControllerStrategy::handleInput(float deltaTime) {
	GLFWgamepadstate state;
    glm::vec3 translate = glm::vec3(0);
    glm::vec3 rotate = glm::vec3(0);
    bool paint = false;

    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
        //ImGui::Text("Button A: %s", (state.buttons[GLFW_GAMEPAD_BUTTON_A] ? "Pressed" : "Released"));
        rotate.y = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
        rotate.x = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
        rotate.z = (state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS ? 1 : 0) - (state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS ? 1 : 0);	
    
        translate.x = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
        translate.y = -state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
        translate.z = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] - state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
    
        if(state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS) {
            controllerMode = ControllerMode::CAMERA;
        }
        if(state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS) {
            controllerMode = ControllerMode::BRUSH;
        }

        paint = state.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS;
    }

    float threshold = 0.2;
    translate = applyDeadzone(translate, threshold);
    rotate = applyDeadzone(rotate, threshold);
    
    if(isAboveDeadzone(translate, threshold)) {
        if(this->controllerMode == ControllerMode::CAMERA) {
            translateCameraCommand->execute(translate*deltaTime);
        } else {
            translateBrushCommand->execute(translate*deltaTime);
        }
    }

    if(isAboveDeadzone(rotate, threshold)) {
        if(this->controllerMode == ControllerMode::CAMERA) {
            rotateCameraCommand->execute(rotate*deltaTime);
        } else {

        }
    }

    if(paint) {
        paintBrushCommand->execute(1.0f);
    }
}

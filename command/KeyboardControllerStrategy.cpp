#include "command.hpp"



KeyboardControllerStrategy::KeyboardControllerStrategy(Camera &camera, LithosApplication &app): camera(camera), app(app) {
    this->translateCameraCommand = new TranslateCameraCommand(camera);
    this->rotateCameraCommand = new RotateCameraCommand(camera);
    this->closeWindowCommand = new CloseWindowCommand(app);
}

void KeyboardControllerStrategy::handleInput(float deltaTime) {
    glm::vec3 translate = glm::vec3(0);
    glm::vec3 rotate = glm::vec3(0);

    if (app.getKeyboardStatus(GLFW_KEY_A) != GLFW_RELEASE) {
        rotate.y = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_D) != GLFW_RELEASE) {
        rotate.y = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_W) != GLFW_RELEASE) {
        rotate.x = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_S) != GLFW_RELEASE) {
        rotate.x = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_Q) != GLFW_RELEASE) {
        rotate.z = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_E) != GLFW_RELEASE) {
        rotate.z = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_RIGHT) != GLFW_RELEASE) {
        translate.x = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_LEFT) != GLFW_RELEASE) {
        translate.x = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_UP) != GLFW_RELEASE) {
        translate.z = 1;
     }
    if (app.getKeyboardStatus(GLFW_KEY_DOWN) != GLFW_RELEASE) {
        translate.z = -1;
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

    if (app.getKeyboardStatus(GLFW_KEY_ESCAPE) != GLFW_RELEASE) {
        closeWindowCommand->execute(1.0f);
    }
}

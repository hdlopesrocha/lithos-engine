#include "command.hpp"



KeyboardControllerStrategy::KeyboardControllerStrategy(Camera &camera, Brush3d &brush3d, LithosApplication &app, Octree &octree): app(app), octree(octree) {
    this->translateCameraCommand = new TranslateCameraCommand(camera);
    this->rotateCameraCommand = new RotateCameraCommand(camera);
    this->closeWindowCommand = new CloseWindowCommand(app);
    this->translateBrushCommand = new TranslateBrushCommand(brush3d, camera);
    this->paintBrushCommand = new PaintBrushCommand(brush3d, octree);
    this->controllerMode = ControllerMode::CAMERA;
}

void KeyboardControllerStrategy::handleInput(float deltaTime) {
    glm::vec3 translate = glm::vec3(0);
    glm::vec3 rotate = glm::vec3(0);
    bool paint = false;
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
    if (app.getKeyboardStatus(GLFW_KEY_PAGE_UP) != GLFW_RELEASE) {
        translate.y = 1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_PAGE_DOWN) != GLFW_RELEASE) {
        translate.y = -1;
    }
    if (app.getKeyboardStatus(GLFW_KEY_1) != GLFW_RELEASE) {
        controllerMode = ControllerMode::CAMERA;
    }
    if (app.getKeyboardStatus(GLFW_KEY_2) != GLFW_RELEASE) {
        controllerMode = ControllerMode::BRUSH;
    }
    if (app.getKeyboardStatus(GLFW_KEY_SPACE) != GLFW_RELEASE) {
        paint = true;
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
        rotateCameraCommand->execute(rotate*deltaTime);
    }

    if (app.getKeyboardStatus(GLFW_KEY_ESCAPE) != GLFW_RELEASE) {
        closeWindowCommand->execute(1.0f);
    }

    if(paint) {
        paintBrushCommand->execute(1.0f);
    }
}

#ifndef COMMAND_HPP
#define COMMAND_HPP
#define KEYBOARD_SIZE 1024

#include "../gl/gl.hpp"
#include "../tools/tools.hpp"

template<typename T> class ICommand {
    public:
    virtual ~ICommand() = default;
    virtual void execute(const T& value) = 0;
};

// Controller Strategy Interface
class ControllerStrategy {
    public:
    virtual ~ControllerStrategy() = default;
    virtual void handleInput(float deltaTime) = 0;
    glm::vec3 applyDeadzone(glm::vec3 input, float threshold);
    bool isAboveDeadzone(const glm::vec3& v, float threshold);
};
    
class TranslateCameraCommand : public ICommand<glm::vec3>{
    Camera &camera;
    public:
    TranslateCameraCommand(Camera &camera);
    void execute(const glm::vec3 &value) override ;
};

class TranslateBrushCommand : public ICommand<glm::vec3>{
    Brush3d &brush3d;
    Camera &camera;
    public:
    TranslateBrushCommand(Brush3d &brush3d, Camera &camera);
    void execute(const glm::vec3 &value) override ;
};

class PaintBrushCommand : public ICommand<float>{
    Brush3d &brush3d;
    Octree &octree;
    DirtyHandler dirtyHandler;
    Simplifier simplifier;
    public:
    PaintBrushCommand(Brush3d &brush3d, Scene &scene);
    void execute(const float &value) override ;
};


class RotateCameraCommand : public ICommand<glm::vec3>{
    Camera &camera;
    public:
    RotateCameraCommand(Camera &camera);
    void execute(const glm::vec3 &value) override ;
};

class CloseWindowCommand : public ICommand<float>{
    LithosApplication &app;
    public:
    CloseWindowCommand(LithosApplication &app);
    void execute(const float &value) override ;
};

enum ControllerMode {
    CAMERA, BRUSH
};

class GamepadControllerStrategy : public ControllerStrategy {
    TranslateCameraCommand * translateCameraCommand;
    TranslateBrushCommand * translateBrushCommand;
    RotateCameraCommand * rotateCameraCommand;
    PaintBrushCommand * paintBrushCommand;
    ControllerMode controllerMode;
    public:
        GamepadControllerStrategy(Camera &camera, Brush3d &brush3d, Scene &scene);
        void handleInput(float deltaTime) override;

};

class KeyboardControllerStrategy : public ControllerStrategy {
    TranslateCameraCommand * translateCameraCommand;
    RotateCameraCommand * rotateCameraCommand;
    CloseWindowCommand * closeWindowCommand;
    TranslateBrushCommand * translateBrushCommand;
    PaintBrushCommand * paintBrushCommand;
    ControllerMode controllerMode;
    LithosApplication &app;
    public:
        KeyboardControllerStrategy(Camera &camera, Brush3d &brush3d, LithosApplication &app, Scene &scene);
        void handleInput(float deltaTime) override;
};


#endif
#ifndef COMMAND_HPP
#define COMMAND_HPP
#define KEYBOARD_SIZE 1024

#include "../gl/gl.hpp"
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



class GamepadControllerStrategy : public ControllerStrategy {
    TranslateCameraCommand * translateCameraCommand;
    RotateCameraCommand * rotateCameraCommand;
    Camera &camera;
    public:
        GamepadControllerStrategy(Camera &camera);
        void handleInput(float deltaTime) override;

};

class KeyboardControllerStrategy : public ControllerStrategy {
    TranslateCameraCommand * translateCameraCommand;
    RotateCameraCommand * rotateCameraCommand;
    CloseWindowCommand * closeWindowCommand;
    Camera &camera;
    LithosApplication &app;
    public:
        KeyboardControllerStrategy(Camera &camera, LithosApplication &app);
        void handleInput(float deltaTime) override;
};


#endif
#ifndef COMMAND_HPP
#define COMMAND_HPP

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
    virtual void handleInput(GLFWwindow* window) = 0;
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

#endif
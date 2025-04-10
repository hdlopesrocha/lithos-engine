#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "../gl/gl.hpp"
// ICommand Interface
class ICommand {
    public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
};
    
// 1D analog input (e.g., trigger)
class IAnalog1DCommand {
    public:
    virtual ~IAnalog1DCommand() = default;
    virtual void execute(float value) = 0;
};

// 2D analog input (e.g., joystick)
class IAnalog2DCommand {
    public:
    virtual ~IAnalog2DCommand() = default;
    virtual void execute(const glm::vec2& input) = 0;
};

// Controller Strategy Interface
class ControllerStrategy {
    public:
    virtual ~ControllerStrategy() = default;
    virtual void handleInput(GLFWwindow* window) = 0;
};
    
#endif
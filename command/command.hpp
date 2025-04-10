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
    
#endif
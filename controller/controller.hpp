#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include "../gl/gl.hpp"
#include "../tools/tools.hpp"
#include "../command/command.hpp"
#include <unordered_map>


// Controller Strategy Interface
class ControllerStrategy {
    public:
    virtual ~ControllerStrategy() = default;
    virtual void handleInput(float deltaTime) = 0;
    glm::vec3 applyDeadzone(glm::vec3 input, float threshold);
    bool isAboveDeadzone(const glm::vec3& v, float threshold);
};
  

enum ControllerMode {
    CAMERA, BRUSH
};

class GamepadControllerStrategy : public ControllerStrategy {
   	EventManager &eventManager;
    std::unordered_map<int, float> keyDuration;
    std::unordered_map<int, bool> keyWasPressed;

    public:
        GamepadControllerStrategy(EventManager &eventManager);
        void handleInput(float deltaTime) override;
        float tick(int key, float deltaTime);

};

class KeyboardControllerStrategy : public ControllerStrategy {
    LithosApplication &app;
   	EventManager &eventManager;
    std::unordered_map<int, bool> keyWasPressed;
    std::unordered_map<int, float> keyDuration;

    public:
        KeyboardControllerStrategy(LithosApplication &app, EventManager &eventManager);
        void handleInput(float deltaTime) override;
        float tick(int key, float deltaTime);
};


#endif
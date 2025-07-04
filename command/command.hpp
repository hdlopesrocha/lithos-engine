#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "../gl/gl.hpp"
#include "../tools/tools.hpp"

template<typename T> class ICommand {
    public:
    virtual ~ICommand() = default;
    virtual void execute(T value) = 0;
};
    
class TranslateCommand : public ICommand<Axis3dEvent>{
    Camera &camera;
    glm::vec3 &vector;
    public:
    TranslateCommand(Camera &camera, glm::vec3 &vector);
    void execute(Axis3dEvent value) override ;
};

class ScaleCommand : public ICommand<Axis3dEvent>{
    Camera &camera;
    glm::vec3 &vector;
    public:
    ScaleCommand(Camera &camera, glm::vec3 &vector);
    void execute(Axis3dEvent value) override ;
};

class PaintBrushCommand : public ICommand<Event>{
    Brush3d &brush3d;
    Octree &octree;
    Simplifier simplifier;
    public:
    PaintBrushCommand(Brush3d &brush3d, Scene &scene);
    void execute(Event event) override ;
};


class RotateCommand : public ICommand<Axis3dEvent>{
    Camera &camera;
    glm::quat &quaternion;
    public:
    RotateCommand(Camera &camera, glm::quat &quaternion);
    void execute(Axis3dEvent value) override ;
};

class CloseWindowCommand : public ICommand<Event>{
    LithosApplication &app;
    public:
    CloseWindowCommand(LithosApplication &app);
    void execute(Event value) override ;
};

template<typename T> class EventTriggerCommand : public EventHandler<T> {
    ICommand<T> *command;
    public:
    EventTriggerCommand(ICommand<T> *command) : command(command) {}
    void handle(T value) override {
        command->execute(value);
    }
};

template<typename T> class ControlBrushCommand : public ICommand<T> {
    BrushContext &brushContext;
    public:
    ControlBrushCommand(BrushContext &brushContext) : brushContext(brushContext) {}
    void execute(T value) override {
        brushContext.handleEvent( value);
    }
};

#endif
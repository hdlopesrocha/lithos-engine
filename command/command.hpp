#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "../gl/gl.hpp"
#include "../tools/tools.hpp"

template<typename T> class ICommand {
    public:
    virtual ~ICommand() = default;
    virtual void execute(T value) = 0;
};
    
class TranslateCommand : public ICommand<TimedAttribute<glm::vec3>>{
    Camera &camera;
    glm::vec3 &vector;
    public:
    TranslateCommand(Camera &camera, glm::vec3 &vector);
    void execute(TimedAttribute<glm::vec3> value) override ;
};

class PaintBrushCommand : public ICommand<float>{
    Brush3d &brush3d;
    Octree &octree;
    Simplifier simplifier;
    public:
    PaintBrushCommand(Brush3d &brush3d, Scene &scene);
    void execute(float value) override ;
};


class RotateCommand : public ICommand<TimedAttribute<glm::vec3>>{
    Camera &camera;
    glm::quat &quaternion;
    public:
    RotateCommand(Camera &camera, glm::quat &quaternion);
    void execute(TimedAttribute<glm::vec3> value) override ;
};

class CloseWindowCommand : public ICommand<float>{
    LithosApplication &app;
    public:
    CloseWindowCommand(LithosApplication &app);
    void execute(float value) override ;
};



#endif
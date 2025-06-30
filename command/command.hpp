#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "../gl/gl.hpp"
#include "../tools/tools.hpp"

template<typename T> class ICommand {
    public:
    virtual ~ICommand() = default;
    virtual void execute(T& value) = 0;
};
    
class TranslateCameraCommand : public ICommand<glm::vec3>{
    Camera &camera;
    public:
    TranslateCameraCommand(Camera &camera);
    void execute(glm::vec3 &value) override ;
};

class TranslateBrushCommand : public ICommand<glm::vec3>{
    Brush3d &brush3d;
    Camera &camera;
    public:
    TranslateBrushCommand(Brush3d &brush3d, Camera &camera);
    void execute(glm::vec3 &value) override ;
};

class PaintBrushCommand : public ICommand<float>{
    Brush3d &brush3d;
    Octree &octree;
    Simplifier simplifier;
    public:
    PaintBrushCommand(Brush3d &brush3d, Scene &scene);
    void execute(float &value) override ;
};


class RotateCameraCommand : public ICommand<glm::vec3>{
    Camera &camera;
    public:
    RotateCameraCommand(Camera &camera);
    void execute(glm::vec3 &value) override ;
};

class CloseWindowCommand : public ICommand<float>{
    LithosApplication &app;
    public:
    CloseWindowCommand(LithosApplication &app);
    void execute(float &value) override ;
};



#endif
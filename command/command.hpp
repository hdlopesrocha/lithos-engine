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
    TranslateCameraCommand(Camera &camera) : camera(camera) {

    }

    void execute(const glm::vec3 &value) override {
        glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f)*camera.quaternion;
		glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f)*camera.quaternion;
		glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, -1.0f)*camera.quaternion;

		camera.position += camera.translationSensitivity*(value.x*xAxis + value.y*yAxis + value.z * zAxis );
    }
};

class RotateCameraCommand : public ICommand<glm::vec3>{
    Camera &camera;
    public:
    RotateCameraCommand(Camera &camera) : camera(camera) {

    }

    void execute(const glm::vec3 &value) override {
        glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f);

        camera.quaternion = glm::angleAxis(camera.rotationSensitivity*value.x, xAxis)*camera.quaternion;
		camera.quaternion = glm::angleAxis(camera.rotationSensitivity*value.y, yAxis)*camera.quaternion;
		camera.quaternion = glm::angleAxis(camera.rotationSensitivity*value.z, zAxis)*camera.quaternion;
    }
};

#endif
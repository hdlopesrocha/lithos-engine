#ifndef GL_HPP
#define GL_HPP


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define KEYBOARD_SIZE 1024
#define VERTEX_BUFFER_BIND_ID 0
#define INSTANCE_BUFFER_BIND_ID 1
#define MAX_FRAMES_IN_FLIGHT 2
#define WIDTH 1280
#define HEIGHT 720
#define DEBUG 1
#define NDEBUG 1

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <vector>
#include <array>
#include <map>
#include <cstring>
#include <set>
#include <algorithm>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <chrono>
#include "rapidjson/document.h"
#include <sstream>
#include <string>
#include "../math/math.hpp"

struct InstanceData {
	glm::mat4 model;
};

struct UniformBufferObject {
	glm::mat4 view;
	glm::mat4 projection;
};


struct IndexBufferObject {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    long indicesCount;
};

struct InstanceBufferObject {
};

struct IndirectDraw {
	std::vector<InstanceData> instances;
    IndexBufferObject * indexBufferObject;
};


typedef uint32_t Image;

struct Model3D {
     std::map<std::string, IndexBufferObject> buffers;
};

struct Camera {
    glm::mat4 projection;
    glm::mat4 view;
    glm::quat quaternion;
    glm::vec3 position;
};

class LithosApplication {
	
private: 
	int initWindow();
    void mainLoop();
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    GLFWwindow* window;
    bool alive = true;
	int keyboard[KEYBOARD_SIZE];

public:
    virtual void setup() = 0;
    virtual void draw() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void clean() = 0;
    void run();
    void close();
    Image loadTextureImage(const std::string& filename);
    int getKeyboardStatus(int key);
    int getWidth();
    int getHeight();




    std::string readFile(const std::string& filePath);
    GLuint compileShader(const std::string& shaderCode, GLenum shaderType);
    GLuint createShaderProgram(GLuint vertexShader, GLuint fragmentShader, GLuint tcs, GLuint tes);

};


#endif

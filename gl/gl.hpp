#ifndef GL_HPP
#define GL_HPP


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define KEYBOARD_SIZE 1024
#define VERTEX_BUFFER_BIND_ID 0
#define INSTANCE_BUFFER_BIND_ID 1
#define MAX_FRAMES_IN_FLIGHT 2
#define WIDTH 1680
#define HEIGHT 1050
#define DEBUG 1
#define NDEBUG 1
#include "../dependencies/imgui/imgui.h"
#include "../dependencies/imgui/imgui_impl_glfw.h"
#include "../dependencies/imgui/imgui_impl_opengl3.h"
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
#include <chrono>
#include <sstream>
#include <string>
#include "../math/math.hpp"
#include <filesystem>
#include <iostream>
#include <ranges>
#include <initializer_list>

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


typedef GLuint TextureArray;
typedef GLuint TextureImage;

struct Model3D {
     std::map<std::string, IndexBufferObject> buffers;
};

struct Camera {
    glm::mat4 projection;
    glm::mat4 view;
    glm::quat quaternion;
    glm::vec3 position;

    glm::mat4 getMVP(glm::mat4 m) {
		return projection * view * m;
	}

};

struct DirectionalLight {
    glm::vec3 direction;
    glm::mat4 projection;
    glm::mat4 view;    
 
    glm::mat4 getMVP(glm::mat4 m) {
		return projection * view * m;
	}
};

struct RenderBuffer {
    GLuint frameBuffer;
    GLuint colorTexture;
    GLuint depthTexture;
    int width;
    int height;
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
    GLint originalFrameBuffer;
    int framesPerSecond = 0;

    virtual void setup() = 0;
    virtual void draw3d() = 0;
    virtual void draw2d() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void clean() = 0;
    void run();
    void close();
    TextureArray loadTextureArray(const std::string& color, const std::string& normal, const std::string& bump);
    TextureImage loadTextureImage(const std::string& color);
    int getKeyboardStatus(int key);
    int getWidth();
    int getHeight();
    GLFWwindow* getWindow();

    std::string readFile(const std::string& filePath);
    GLuint compileShader(const std::string& shaderCode, GLenum shaderType);
    GLuint createShaderProgram(std::initializer_list<GLuint> shaders);
};

class DrawableGeometry {
	public:
	GLuint vao, vbo, ebo;
	int indices;

	DrawableGeometry(Geometry * t);
    ~DrawableGeometry();
    void draw(uint mode);
    static GLuint create2DVAO(float x, float y, float w, float h);

};

class OctreeProcessor : public IteratorHandler{
	Octree * tree;
	Geometry chunk;
	Frustum frustum;
	int geometryType;
    int drawableType;
	float simplificationAngle;
    float simplificationDistance;
    bool simplificationTexturing;

    public: 
		int loaded = 0;
		int geometryLevel;
        glm::vec3 cameraPosition;
        int * triangles;
		OctreeProcessor(Octree * tree, int * triangles, int drawableType, int geometryLevel, float simplificationAngle, float simplificationDistance, bool simplificationTexturing);

		void update(glm::mat4 m);
		void * before(int level, OctreeNode * node, BoundingCube cube, void * context);
		void after(int level, OctreeNode * node, BoundingCube cube, void * context);
		bool test(int level, OctreeNode * node, BoundingCube cube, void * context);
        OctreeNode * getChild(OctreeNode * node, int index);
		void getOrder(OctreeNode * node, BoundingCube cube, int * order);

};


class OctreeRenderer : public IteratorHandler{
	Octree * tree;
	Geometry chunk;
	Frustum frustum;
	int geometryType;
    int drawableType;

    public: 
        uint mode;
		int geometryLevel;
        glm::vec3 cameraPosition;
		OctreeRenderer(Octree * tree, int drawableType, int geometryLevel);

		void update(glm::mat4 m);
		void * before(int level, OctreeNode * node, BoundingCube cube, void * context);
		void after(int level, OctreeNode * node, BoundingCube cube, void * context);
		bool test(int level, OctreeNode * node, BoundingCube cube, void * context);
        OctreeNode * getChild(OctreeNode * node, int index);
		void getOrder(OctreeNode * node, BoundingCube cube, int * order);

};

class Texture {
	public:
	int index;
	TextureArray texture;

	Texture();
	Texture(TextureArray texture);

    static int bindTexture(GLuint program, GLuint type, int activeTexture, std::string objectName, GLuint texture);
    static int bindTextures(GLuint program, GLuint type,int activeTexture, std::string arrayName, std::vector<Texture*> * ts);

};


class Brush {
    public:
    Texture * texture;
	float parallaxScale;
	float parallaxMinLayers;
	float parallaxMaxLayers;
	float parallaxFade;
	float parallaxRefine;
	float shininess;
	float specularStrength;
    float refractiveIndex;
	glm::vec2 textureScale;


    Brush(Texture * texture);
    Brush(Texture * texture, glm::vec2 textureScale,float parallaxScale, float parallaxMinLayers, float parallaxMaxLayers, float parallaxFade, float parallaxRefine, float shininess, float specularStrength, float refractiveIndex);

    static void bindBrushes(GLuint program, std::vector<Brush*> * brushes);
    static void bindBrush(GLuint program, std::string objectName, Brush * brush);

};

class TextureMixer {
    RenderBuffer textureMixerBuffer;
    std::vector<Texture*> * textures;
    GLuint program;
    GLuint previewVao;

    public:

    float perlinTime;
    int perlinScale;
    int perlinLacunarity;
    int perlinIterations;
    float brightness;
    float contrast;
    int baseTextureIndex;
    int overlayTextureIndex;

    TextureMixer(int width, int height, GLuint program, std::vector<Texture*> * textures);
    TextureArray getTexture();
    void mix(int baseTexture, int overlayTexture);
    void mix();
};

class AnimatedTexture {
    RenderBuffer textureMixerBuffer;
    GLuint program;
    GLuint previewVao;

    public:

    int perlinScale;
    int perlinLacunarity;
    int perlinIterations;
    float brightness;
    float contrast;
    glm::vec4 color;
    AnimatedTexture(int width, int height, GLuint program);
    TextureArray getTexture();
    void animate(float time);
};

class Settings {
    public:
        bool parallaxEnabled;
        bool shadowEnabled;
        bool debugEnabled;
        bool lightEnabled;
        bool wireFrameEnabled;

        Settings();

};

struct Tile {
    public:
    glm::vec2 size;
    glm::vec2 offset;
    
    Tile(glm::vec2 size, glm::vec2 offset);
};

struct TileDraw {
    public:
    glm::vec2 size;
    glm::vec2 offset;
    float angle;
    uint index;
    
    TileDraw(uint index,glm::vec2 size, glm::vec2 offset, float angle);
};


class Vegetation3d {
    public:
    DrawableGeometry * drawable;

    Vegetation3d();
};

class AtlasTexture: public Texture {
    public:
    using Texture::Texture;
    std::vector<Tile> tiles;
};

class AtlasDrawer {
    RenderBuffer renderBuffer;
    GLuint program;
    GLuint viewVao;
    GLuint samplerLoc;
    GLuint layerLoc; 
    GLuint modelLoc; 
    GLuint tileOffsetLoc;
    GLuint tileSizeLoc;
    public:
    AtlasDrawer(GLuint program, int width, int height);
    TextureArray getTexture();
    void draw(AtlasTexture * atlas,  std::vector<TileDraw> draws);
};

GLuint createTextureArray(int width, int height, int layers); 
RenderBuffer createMultiLayerRenderFrameBuffer(int width, int height, int layers);
RenderBuffer createDepthFrameBuffer(int width, int height);
RenderBuffer createRenderFrameBuffer(int width, int height);
RenderBuffer createRenderFrameBufferWithoutDepth(int width, int height);
#endif

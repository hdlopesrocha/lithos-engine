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


#define TYPE_INSTANCE_VEGETATION_DRAWABLE 1
#define TYPE_INSTANCE_SOLID_DRAWABLE 2
#define TYPE_INSTANCE_LIQUID_DRAWABLE 3
#define TYPE_INSTANCE_SHADOW_DRAWABLE 4


#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
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

#define PARALLAX_FLAG       0,0x01  
#define SHADOW_FLAG         0,0x02  
#define DEBUG_FLAG          0,0x04  
#define LIGHT_FLAG          0,0x08  
#define TRIPLANAR_FLAG      0,0x10  
#define DEPTH_FLAG          0,0x20  
#define OVERRIDE_FLAG       0,0x40  
#define TESSELATION_FLAG    0,0x80  

#define OPACITY_FLAG      1,0x01  

#define OVERRIDE_TEXTURE_FLAG 0xff000000

#pragma pack(16)  // Ensure 16-byte alignment for UBO
struct UniformBlock {
    glm::mat4 world;          
    glm::mat4 viewProjection;  
    glm::mat4 matrixShadow;      
    glm::vec4 lightDirection;   
    glm::vec4 cameraPosition;   
    glm::vec4 floatData;  
    glm::uvec4 uintData;      

    public:
    static std::string toString(UniformBlock * block);
    void set(uint index, uint flag, bool value) {
        if(value){
            uintData[index] |= flag;
        }else {
            uintData[index] &= ~flag;
        }
    }
};
#pragma pack()  // Reset to default packing


struct ProgramData {
	public:
	GLuint ubo;

	ProgramData();
	void uniform(UniformBlock * block);
};

typedef GLuint TextureArray;
typedef GLuint TextureImage;

struct Camera {
    glm::mat4 projection;
    glm::mat4 view;
    glm::quat quaternion;
    glm::vec3 position;

    glm::mat4 getVP() {
		return projection * view;
	}

};

struct DirectionalLight {
    glm::vec3 direction;
    glm::mat4 projection;
    glm::mat4 view;    
 
    glm::mat4 getVP() {
		return projection * view;
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
    TextureArray loadTextureArray(std::initializer_list<std::string> fns);
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
    bool createInstances;
    int simplification;
    public: 
		int loaded = 0;
		int geometryLevel;
        glm::vec3 cameraPosition;
        int * instancesCount;
		OctreeProcessor(Octree * tree, int * instancesCount, int drawableType, int geometryLevel, float simplificationAngle, float simplificationDistance, bool simplificationTexturing, bool createInstances, int simplification);

		void update(glm::mat4 m);
		void * before(int level, OctreeNode * node, BoundingCube cube, void * context);
		void after(int level, OctreeNode * node, BoundingCube cube, void * context);
		bool test(int level, OctreeNode * node, BoundingCube cube, void * context);
        OctreeNode * getChild(OctreeNode * node, int index);
		void getOrder(OctreeNode * node, BoundingCube cube, int * order);

};

struct InstanceData {
    public:
    glm::mat4 matrix;
    float shift;

    InstanceData(glm::mat4 matrix,  float shift) {
        this->matrix = matrix;
        this->shift = shift;
    }
};


class OctreeNodeTriangleInstanceBuilder : public OctreeNodeTriangleHandler {

	public: 
	OctreeNode ** corners;
	std::vector<InstanceData> * instances;
    int pointsPerTriangle;

	using OctreeNodeTriangleHandler::OctreeNodeTriangleHandler;
	OctreeNodeTriangleInstanceBuilder(Geometry * chunk, int * count,OctreeNode ** corners,std::vector<InstanceData> * instances, int pointsPerTriangle);
	void handle(OctreeNode* c0,OctreeNode* c1,OctreeNode* c2, bool sign);

};

class DrawableInstanceGeometry {
	public:
	GLuint vertexArrayObject = 0u;
	GLuint vertexBuffer = 0u;
	GLuint indexBuffer = 0u;
	GLuint instanceBuffer = 0u;

	int indicesCount;
    int instancesCount;

	DrawableInstanceGeometry(Geometry * t, std::vector<InstanceData> * instances);
    ~DrawableInstanceGeometry();
    void draw(uint mode);
};

class InstanceBuilder : public IteratorHandler{
	Octree * tree;
	Geometry chunk;
    uint mode;
    int lod;

    public: 
        int instanceCount = 0;
        std::vector<InstanceData> instances;
		InstanceBuilder(Octree * tree, int lod);

		void * before(int level, OctreeNode * node, BoundingCube cube, void * context);
		void after(int level, OctreeNode * node, BoundingCube cube, void * context);
		bool test(int level, OctreeNode * node, BoundingCube cube, void * context);
        OctreeNode * getChild(OctreeNode * node, int index);
		void getOrder(OctreeNode * node, BoundingCube cube, int * order);

};

class Texture {
	public:
	TextureArray texture;

	Texture();
	Texture(TextureArray texture);

    static int bindTexture(GLuint program, GLuint type, int activeTexture, std::string objectName, GLuint texture);
    static int bindTexture(GLuint program, GLuint type, int activeTexture, GLuint location, GLuint texture);

    static int bindTextures(GLuint program, GLuint type,int activeTexture, std::string arrayName, std::vector<Texture*> * ts);

};


class Brush {
    public:
    uint textureIndex;
    uint brushIndex;
	float parallaxScale;
	float parallaxMinLayers;
	float parallaxMaxLayers;
	float parallaxFade;
	float parallaxRefine;
	float shininess;
	float specularStrength;
    float refractiveIndex;
	glm::vec2 textureScale;


    Brush(uint textureIndex);
    Brush(uint textureIndex, glm::vec2 textureScale,float parallaxScale, float parallaxMinLayers, float parallaxMaxLayers, float parallaxFade, float parallaxRefine, float shininess, float specularStrength, float refractiveIndex);

    static void bindBrushes(GLuint program, std::string objectName, std::string mapName, std::vector<Brush*> * brushes);
    static void bindBrush(GLuint program, std::string objectName, std::string textureMap, Brush * brush);

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
        bool tesselationEnabled;
        bool opacityEnabled;
        bool billboardEnabled;

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
    glm::vec2 pivot;
    float angle;
    uint index;
    
    TileDraw(uint index,glm::vec2 size, glm::vec2 offset, glm::vec2 pivot, float angle);
};


class Vegetation3d : public Geometry {
    public:
    Vegetation3d();
    DrawableInstanceGeometry * createDrawable(std::vector<InstanceData> * instances);
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
    GLuint filterLoc;
    std::vector<AtlasTexture*> * atlasTextures;
    
    public:
    bool filterEnabled = true;
    int atlasIndex;
    std::vector<TileDraw> draws;
    AtlasDrawer(GLuint program, int width, int height, std::vector<AtlasTexture*> * atlasTextures);
    TextureArray getTexture();
    void draw(int atlasIndex,  std::vector<TileDraw> draws);
    void draw();
};

class OctreeInstanceRenderer : public IteratorHandler{
	Octree * tree;
	Geometry chunk;
	Frustum frustum;
	int geometryType;
    int drawableType;
    uint mode;

    
    public: 
        int * instances;
		int geometryLevel;
        glm::vec3 cameraPosition;
		OctreeInstanceRenderer(Octree * tree, int * instances, int mode, int drawableType, int geometryLevel);

		void update(glm::mat4 m);
		void * before(int level, OctreeNode * node, BoundingCube cube, void * context);
		void after(int level, OctreeNode * node, BoundingCube cube, void * context);
		bool test(int level, OctreeNode * node, BoundingCube cube, void * context);
        OctreeNode * getChild(OctreeNode * node, int index);
		void getOrder(OctreeNode * node, BoundingCube cube, int * order);

};

GLuint createTextureArray(int width, int height, int layers); 
RenderBuffer createMultiLayerRenderFrameBuffer(int width, int height, int layers);
RenderBuffer createDepthFrameBuffer(int width, int height);
RenderBuffer createRenderFrameBuffer(int width, int height);
RenderBuffer createRenderFrameBufferWithoutDepth(int width, int height);
#endif

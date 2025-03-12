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
#define BILLBOARD_FLAG      1,0x02 

#define OVERRIDE_TEXTURE_FLAG 0xff000000
#define SHADOW_MATRIX_COUNT 3

class TextureProperties;

struct ProgramData {
	public:
	GLuint ubo;

	ProgramData();
};

#pragma pack(16)  // Ensure 16-byte alignment for UBO
struct UniformBlock {
    glm::mat4 world;          
    glm::mat4 viewProjection;  
    glm::mat4 matrixShadow[SHADOW_MATRIX_COUNT];      
    glm::vec4 lightDirection;   
    glm::vec4 cameraPosition;   
    glm::vec4 floatData;  
    glm::uvec4 uintData;      

    public:
    static std::string toString(UniformBlock * block);
    void set(uint index, uint flag, bool value);
	static void uniform(void * block, size_t size, GLuint bindingIndex, ProgramData * data);
};
#pragma pack()  // Reset to default packing

#pragma pack(16)  // Ensure 16-byte alignment for UBO
struct UniformBlockBrush {
    public:
    float parallaxScale;
    float parallaxMinLayers;
    float parallaxMaxLayers;
    float parallaxFade;
    float parallaxRefine;
    float shininess;
    float specularStrength;
    float refractiveIndex;
    glm::vec2 textureScale;

    UniformBlockBrush();
    UniformBlockBrush(glm::vec2 textureScale);
    UniformBlockBrush(glm::vec2 textureScale,float parallaxScale, float parallaxMinLayers, float parallaxMaxLayers, float parallaxFade, float parallaxRefine, float shininess, float specularStrength, float refractiveIndex);
    static void uniform(GLuint program, std::vector<TextureProperties> *brushes, std::string objectName, std::string textureMap);
    static void uniform(GLuint program, TextureProperties * brush, std::string objectName, std::string textureMap, int index);

    static std::string toString(UniformBlockBrush * block);
};
#pragma pack()  // Reset to default packing


class TextureProperties {
    public:
    uint textureIndex;
    UniformBlockBrush brush;


    TextureProperties(uint textureIndex, UniformBlockBrush brush);

    
};

struct TextureArray {
    GLuint index;
    GLuint channel;
};

struct TextureImage {
    bool mipmapEnabled;
    GLuint index;
    GLuint channel;
};



class Camera {
    public:
    glm::mat4 projection;
    glm::mat4 view;
    glm::quat quaternion;
    glm::vec3 position;
    float near;
    float far;

    Camera(float near, float far);
    glm::vec3 getCameraDirection();
    glm::mat4 getVP();
};

struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 position;

    void updatePosition(const Camera &camera) {
        // Calculate a distance for the light based on the far plane, used to capture the scene.
        float dist = camera.far / 4.0f;
        this->position = camera.position -direction*dist;
    }

    glm::mat4 getVP(Camera * camera, float orthoSize, float near, float far) {

        
        // Create an orthographic projection matrix for shadow mapping.
        glm::mat4 projection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, near, far);
        


        // Set up the view matrix so that the light looks at the camera's position,
        // but it is positioned along the light's direction, offset by dist to capture the scene's visible range.

        glm::vec3 lightLootAt = camera->position;

        glm::mat4 view = glm::lookAt(position, lightLootAt, glm::vec3(0.0f, 1.0f, 0.0f));
        
        // Return the combined projection * view matrix for shadow mapping.
        return projection * view;
    }
};

struct RenderBuffer {
    GLuint frameBuffer;
    TextureImage colorTexture;
    TextureImage depthTexture;
    int width;
    int height;
};

struct MultiLayerRenderBuffer {
    GLuint frameBuffer;
    TextureArray colorTexture;
    TextureImage depthTexture;
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
    TextureImage loadTextureImage(const std::string& color, bool mipmapEnabled);
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

    int drawableType;
	float simplificationAngle;
    float simplificationDistance;
    bool simplificationTexturing;
    bool createInstances;
    int simplification;
    public: 
		int loadCount = 0;
		int geometryLevel;
        glm::vec3 cameraPosition;
        int * instancesCount;
		OctreeProcessor(Octree * tree, int * instancesCount, int drawableType, int geometryLevel, float simplificationAngle, float simplificationDistance, bool simplificationTexturing, bool createInstances, int simplification);

		void * before(int level, int height, OctreeNode * node, const BoundingCube &cube, void * context) override;
		void after(int level, int height, OctreeNode * node, const BoundingCube &cube, void * context) override;
		bool test(int level, int height, OctreeNode * node, const BoundingCube &cube, void * context) override;
		void getOrder(const BoundingCube &cube, int * order) override;

};


struct InstanceData {
    public:
    float shift;
    glm::mat4 matrix;

    InstanceData(glm::mat4 matrix,  float shift) {
        this->matrix = matrix;
        this->shift = shift;
    }
};

struct PreLoadedGeometry {
    public:
    Geometry * geometry;
    std::vector<InstanceData> instances;
    glm::vec3 center;
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

    glm::vec3 center;
	int indicesCount;
    int instancesCount;

	DrawableInstanceGeometry(Geometry * t, std::vector<InstanceData> * instances);
    ~DrawableInstanceGeometry();
    void draw(uint mode);
    void draw(uint mode, float amount);
};

class InstanceBuilder : public IteratorHandler{
	Octree * tree;
	Geometry chunk;
    uint mode;
    int lod;

    public: 
        int instanceCount = 0;
        std::vector<InstanceData> * instances;
		InstanceBuilder(Octree * tree, int lod, std::vector<InstanceData> * instances);

		void * before(int level, int height, OctreeNode * node, const BoundingCube &cube, void * context) override;
		void after(int level, int height, OctreeNode * node, const BoundingCube &cube, void * context) override;
		bool test(int level, int height, OctreeNode * node, const BoundingCube &cube, void * context) override;
		void getOrder(const BoundingCube &cube, int * order) override;

};

class Texture {
	public:
    static int bindTexture(GLuint program, int activeTexture, GLuint location, TextureImage texture);
    static int bindTexture(GLuint program, int activeTexture, std::string objectName, TextureImage texture);
    static int bindTexture(GLuint program, int activeTexture, std::string objectName, TextureArray texture);
    static int bindTexture(GLuint program, int activeTexture, GLuint location, TextureArray texture);
};





struct TextureLayers {
    TextureArray textures[3];
    int count;
};

class TextureBlitter {
    GLuint program;
    GLuint programCopy;
    GLuint resizeVao;
    std::map<GLuint, MultiLayerRenderBuffer> buffers;

    public:
    TextureBlitter(GLuint program, int width, int height, std::initializer_list<GLuint> formats);
    void blit(MultiLayerRenderBuffer * sourceBuffer, int sourceIndex, TextureArray * targetBuffer, int targetIndex);
};

struct MixerParams {
    public:
    int targetTexture = 0;
    int baseTexture = 0; 
    int overlayTexture =0;
    float perlinTime = 0;
    int perlinScale= 16;
    int perlinLacunarity= 2;
    int perlinIterations =5;
    float brightness = 0;
    float contrast =10;
    MixerParams(int targetTexture,int baseTexture, int overlayTexture);
};

class TextureMixer {
    TextureLayers * layers;

    MultiLayerRenderBuffer textureMixerBuffer;
    GLuint program;
    TextureBlitter * blitter;
    GLuint previewVao;

    public:
    TextureMixer(int width, int height, GLuint program, TextureLayers * layers, TextureBlitter * blitter);
    TextureArray getTexture();
    void mix(MixerParams &params);
};

class AnimateParams {
    public:
    int perlinScale;
    int perlinLacunarity;
    int perlinIterations;
    float brightness;
    float contrast;
    int targetTexture = 0;
    glm::vec4 color;

    AnimateParams(int targetTexture);

};

class AnimatedTexture {
    MultiLayerRenderBuffer textureMixerBuffer;
    GLuint program;

    GLuint previewVao;
    TextureLayers * layers;
    TextureBlitter * blitter;
    public:

    AnimatedTexture(int width, int height, GLuint program, TextureLayers * layers, TextureBlitter * blitter);
    TextureArray getTexture();
    void animate(float time, AnimateParams &params);
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
        bool overrideEnabled;
        uint billboardRange;
        uint overrideTexture;
        uint debugMode;
        float blendSharpness;
        float parallaxDistance;
        float parallaxPower;
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
};

class AtlasTexture {
    public:
    std::vector<Tile> tiles;
};

struct AtlasParams {
    public:
    AtlasTexture * atlasTexture;
    int targetTexture = 0;
    int sourceTexture = 0;
    std::vector<TileDraw> draws;
    AtlasParams();
    AtlasParams(int sourceTexture, int targetTexture, AtlasTexture * atlasTexture);

};

class AtlasDrawer {
    MultiLayerRenderBuffer renderBuffer;
    GLuint program;
    GLuint viewVao;
    GLuint layerLoc; 
    GLuint modelLoc; 
    GLuint tileOffsetLoc;
    GLuint tileSizeLoc;
    GLuint filterLoc;
    GLuint atlasTextureLoc;
    TextureLayers * sourceLayers;
    TextureLayers * targetLayers;
    TextureBlitter * blitter;

    public:
    bool filterEnabled = true;
    AtlasDrawer(GLuint program, int width, int height, TextureLayers * sourceLayers, TextureLayers * targetLayers, TextureBlitter * blitter);
    TextureArray getTexture();
    void draw(AtlasParams &params);
};

class ImpostorParams {
    public:
    int targetTexture = 0;
    DrawableInstanceGeometry * mesh;
    ImpostorParams(int targetTexture, DrawableInstanceGeometry * mesh);

};


class ImpostorDrawer {
    GLuint program;
    MultiLayerRenderBuffer renderBuffer;
    int width; 
    int height;
    TextureBlitter * blitter;
    TextureLayers* sourceLayers;
    TextureLayers * targetLayers;
    public:
    ImpostorDrawer(GLuint program, int width, int height, TextureLayers* sourceLayers, TextureLayers * targetLayers, TextureBlitter * blitter);
    void draw(ImpostorParams &params);
    TextureArray getTexture();
};
void blitTextureArray(GLuint programCopy, MultiLayerRenderBuffer buffer, TextureLayers * layers, int index);
void blitRenderBuffer(TextureArray textures[0], TextureLayers layers, RenderBuffer buffer, int sourceIndex, int destinationIndex);
void blitRenderBuffer(TextureArray textures[0], TextureLayers layers, MultiLayerRenderBuffer buffer, int sourceIndex);

void loadTexture(TextureLayers * layers, std::initializer_list<std::string> fns, int index, bool mipMapping);
TextureArray createTextureArray(int width, int height, int layers, GLuint channel); 
MultiLayerRenderBuffer createMultiLayerRenderFrameBuffer(int width, int height, int layers, int attachments, bool depth, GLuint color);
RenderBuffer createDepthFrameBuffer(int width, int height);
RenderBuffer createRenderFrameBuffer(int width, int height, bool depth);
RenderBuffer createRenderFrameBufferWithoutDepth(int width, int height);
#endif

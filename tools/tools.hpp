#ifndef TOOLS_HPP
#define TOOLS_HPP
#include "../math/math.hpp"
#include "../gl/gl.hpp"
#define DISCARD_BRUSH_INDEX -1



class LandBrush : public TextureBrush {
	Brush * underground;
	Brush * grass;
	Brush * sand;
	Brush * softSand;
	Brush * rock;
	Brush * snow;
	Brush * grassMixSand;
	Brush * grassMixSnow;
	Brush * rockMixGrass;
	Brush * rockMixSnow;
	Brush * rockMixSand;

	public: 
	LandBrush(std::vector<Brush*> brushes);
	void paint(Vertex * vertex);
};

class SimpleBrush : public TextureBrush {
	Brush * brush;

	public: 
	SimpleBrush(Brush * brush);
	void paint(Vertex * vertex);
};



class WaterBrush : public TextureBrush {
	Brush * water;

	public: 
	WaterBrush(Brush* water);
	void paint(Vertex * vertex);
};

class InstanceBuffer {
	public:
	std::vector<glm::vec3> positions;
	GLuint vbo;
	
	InstanceBuffer(int size);
};


struct alignas(16) UniformBlock {
    glm::mat4 model;             // 16 bytes
    glm::mat4 modelViewProjection;  // 16 bytes
    glm::mat4 matrixShadow;      // 16 bytes
    glm::vec4 lightDirection;    // 16 bytes
    glm::vec4 cameraPosition;    // 16 bytes
    float time;                  // 4 bytes
    int parallaxEnabled;         // 4 bytes
    int shadowEnabled;           // 4 bytes
    int debugEnabled;            // 4 bytes
    int lightEnabled;            // 4 bytes
    int triplanarEnabled;        // 4 bytes
    int layer;                  // 4 bytes (instead of int)
    int padding;                 // 4 bytes (optional for alignment)

    // Total size: 120 bytes (aligned to 16 bytes)
};

struct ProgramData {
	public:


	bool wireFrameEnabled;
	GLuint ubo;
	GLuint program;
	GLuint depthTextureLoc;
	GLuint underTextureLoc;
	GLuint overrideTextureEnabledLoc;
	GLuint shadowMapLoc;
	GLuint noiseLoc;

	ProgramData(GLuint program);
	void uniform(UniformBlock * block);
};

#endif
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

struct ProgramData {
	public:
	glm::mat4 modelViewProjection;
	glm::mat4 model;
	glm::mat4 matrixShadow;
	glm::vec3 lightDirection;
	glm::vec3 cameraPosition;
	float time;
	bool parallaxEnabled;
	bool shadowEnabled;
	bool debugEnabled;
	bool lightEnabled;
	bool wireFrameEnabled;
	int layer;

	GLuint program;
	GLuint modelLoc;
	GLuint modelViewProjectionLoc;
	GLuint matrixShadowLoc;
	GLuint lightDirectionLoc;
	GLuint lightEnabledLoc;
	GLuint debugEnabledLoc;
	GLuint triplanarEnabledLoc;
	GLuint parallaxEnabledLoc;
	GLuint shadowEnabledLoc;
	GLuint depthTextureLoc;
	GLuint underTextureLoc;
	GLuint cameraPositionLoc;
	GLuint overrideTextureEnabledLoc;
	GLuint shadowMapLoc;
	GLuint noiseLoc;
	GLuint timeLoc;
	GLuint layerLoc;

	ProgramData(GLuint program);
	void uniform();
};

#endif
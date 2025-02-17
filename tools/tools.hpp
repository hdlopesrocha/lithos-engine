#ifndef TOOLS_HPP
#define TOOLS_HPP
#include "../math/math.hpp"
#include "../gl/gl.hpp"
#define DISCARD_BRUSH_INDEX -1



class LandBrush : public TextureBrush {
	uint underground;
	uint grass;
	uint sand;
	uint softSand;
	uint rock;
	uint snow;
	uint grassMixSand;
	uint grassMixSnow;
	uint rockMixGrass;
	uint rockMixSnow;
	uint rockMixSand;

	public: 
	LandBrush(std::vector<Brush*> brushes);
	void paint(Vertex * vertex);
};

class SimpleBrush : public TextureBrush {
	Texture * texture;

	public: 
	SimpleBrush(Texture * texture);
	void paint(Vertex * vertex);
};



class WaterBrush : public TextureBrush {
	Texture * water;
	Texture discardTexture;

	public: 
	WaterBrush(Texture* water);
	void paint(Vertex * vertex);
};


#endif
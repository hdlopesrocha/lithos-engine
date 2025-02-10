#ifndef TOOLS_HPP
#define TOOLS_HPP
#include "../math/math.hpp"
#include "../gl/gl.hpp"
#define DISCARD_BRUSH_INDEX -1



class LandBrush : public TextureBrush {
	Texture * underground;
	Texture * grass;
	Texture * sand;
	Texture * softSand;
	Texture * rock;
	Texture * snow;
	Texture * grassMixSand;
	Texture * grassMixSnow;
	Texture * rockMixGrass;
	Texture * rockMixSnow;
	Texture * rockMixSand;
	Texture discardTexture;

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
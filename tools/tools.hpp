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



#endif
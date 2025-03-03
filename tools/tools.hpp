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
	void paint(Vertex * vertex) const override;
};

class SimpleBrush : public TextureBrush {
	Brush * brush;

	public: 
	SimpleBrush(Brush * brush);
	void paint(Vertex * vertex) const override;
};



class WaterBrush : public TextureBrush {
	Brush *water;

	public: 
	WaterBrush(Brush *water);
	void paint(Vertex * vertex) const override;
};

#include "tools.hpp"

class OctreeContainmentHandler : public ContainmentHandler {
	public:
	Octree * octree;
    const TextureBrush &brush;
	BoundingBox box;

	OctreeContainmentHandler(Octree * octree, BoundingBox box, const TextureBrush &b);
	glm::vec3 getNormal(const glm::vec3 pos) const;
	glm::vec3 getCenter() const override;
	bool contains(const glm::vec3 p) const override;
	bool isContained(const BoundingCube &p) const override;
	ContainmentType check(const BoundingCube &cube) const override;
	Vertex getVertex(const BoundingCube &cube, ContainmentType solid, glm::vec3 previousPoint) const override;
};


#endif
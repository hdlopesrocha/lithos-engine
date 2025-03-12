#ifndef TOOLS_HPP
#define TOOLS_HPP
#include "../math/math.hpp"
#include "../gl/gl.hpp"
#define DISCARD_BRUSH_INDEX -1



class LandBrush : public TexturePainter {
	int underground;
	int grass;
	int sand;
	int softSand;
	int rock;
	int snow;
	int grassMixSand;
	int grassMixSnow;
	int rockMixGrass;
	int rockMixSnow;
	int rockMixSand;

	public: 
	LandBrush();
	void paint(Vertex &vertex) const override;
};

class SimpleBrush : public TexturePainter {
	int brush;

	public: 
	SimpleBrush(int brush);
	void paint(Vertex &vertex) const override;
};



class WaterBrush : public TexturePainter {
	int water;

	public: 
	WaterBrush(int water);
	void paint(Vertex &vertex) const override;
};

#include "tools.hpp"

class OctreeContainmentHandler : public ContainmentHandler {
	public:
	Octree * octree;
    const TexturePainter &brush;
	BoundingBox box;

	OctreeContainmentHandler(Octree * octree, BoundingBox box, const TexturePainter &b);
	glm::vec3 getNormal(const glm::vec3 pos) const;
	glm::vec3 getCenter() const override;
	bool contains(const glm::vec3 p) const override;
	bool isContained(const BoundingCube &p) const override;
	ContainmentType check(const BoundingCube &cube) const override;
	Vertex getVertex(const BoundingCube &cube, ContainmentType solid, glm::vec3 previousPoint) const override;
};


#endif
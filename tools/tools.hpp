#ifndef TOOLS_HPP
#define TOOLS_HPP

#include "../math/math.hpp"
#include "../gl/gl.hpp"
#define DISCARD_BRUSH_INDEX -1

struct OctreeNodeSerialized {
    public:
    glm::vec3 position;
    glm::vec3 normal;
    uint texIndex;
    uint mask;
    ContainmentType solid;
    uint children[8];
};

class OctreeSaver : public IteratorHandler{
	Octree * tree;
    std::ofstream file;
    std::vector<OctreeNodeSerialized*> nodes;

    public: 
		OctreeSaver(Octree * tree, std::string filename);

		void update(glm::mat4 m);
		void * before(int level, OctreeNode * node, BoundingCube cube, void * context);
		void after(int level, OctreeNode * node, BoundingCube cube, void * context);
		bool test(int level, OctreeNode * node, BoundingCube cube, void * context);
        OctreeNode * getChild(OctreeNode * node, int index);
		void getOrder(OctreeNode * node, BoundingCube cube, int * order);
        void close();
};


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
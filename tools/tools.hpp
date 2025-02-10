#ifndef TOOLS_HPP
#define TOOLS_HPP
#include <filesystem>
#include <iostream>
#include <fstream>
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

struct OctreeSerialized {
    public:
    glm::vec3 min;
    float length;
	float minSize;
};


class OctreeFile {
	Octree * tree;
    std::string filename;
    int chunkHeight;

    public: 
		OctreeFile(Octree * tree, std::string filename, int chunkHeight);
        void save();
        void load();

};


class OctreeNodeFile {
	OctreeNode * node;
    std::string filename;

    public: 
		OctreeNodeFile(OctreeNode * node, std::string filename);
        void save();
        OctreeNode * load();
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

void ensureFolderExists(const std::string& folder);

#endif
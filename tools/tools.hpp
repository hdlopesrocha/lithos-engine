#ifndef TOOLS_HPP
#define TOOLS_HPP

#define TYPE_INSTANCE_VEGETATION_DRAWABLE 0x1
#define TYPE_INSTANCE_SOLID_DRAWABLE 0x2
#define TYPE_INSTANCE_LIQUID_DRAWABLE 0x4
#define TYPE_INSTANCE_OCTREE_DRAWABLE 0x8

#include "../gl/gl.hpp"
#include "../space/space.hpp"
#include <algorithm>
#include <random>

class WaveSurface : public HeightFunction {
    float amplitude;// = 10;
    float offset;// = -36;
    float frequency;// = 1.0/10.0;

	WaveSurface(float amplitude, float offset , float frequency);

	float getHeightAt(float x, float z) const override;
};

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


class DerivativeLandBrush : public TexturePainter {
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
	DerivativeLandBrush();
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

class OctreeContainmentHandler : public ContainmentHandler {
	public:
	Octree * octree;
    const TexturePainter &brush;
	BoundingBox box;

	OctreeContainmentHandler(Octree * octree, BoundingBox box, const TexturePainter &b);
	glm::vec3 getNormal(const glm::vec3 pos) const;
	float distance(const glm::vec3 p) const override;
	bool isContained(const BoundingCube &p) const override;
	ContainmentType check(const BoundingCube &cube) const override;
	Vertex getVertex(const BoundingCube &cube, glm::vec3 previousPoint) const override;
};

class VegetationInstanceBuilderHandler : public InstanceBuilderHandler {
	public:
	float pointsPerArea;
	float scale;
	Octree * tree;
	VegetationInstanceBuilderHandler(Octree * tree, float pointsPerArea, float scale);

	void handle(OctreeNodeData &data, std::vector<InstanceData> * instances) override;
};

class OctreeInstanceBuilderHandler : public InstanceBuilderHandler {
	public:

	OctreeInstanceBuilderHandler();

	void handle(OctreeNodeData &data, std::vector<InstanceData> * instances) override;
};

class VegetationGeometryBuilder : public GeometryBuilder {
    public:
    Geometry * geometry;
    Octree * tree;
    InstanceBuilderHandler * handler;
	long * instancesCount;
    VegetationGeometryBuilder(Octree * tree, InstanceBuilderHandler * handler);
    ~VegetationGeometryBuilder();

	InstanceGeometry * build(OctreeNodeData &params) override;
};

class OctreeGeometryBuilder : public GeometryBuilder {
    public:
    Geometry * geometry;
    InstanceBuilderHandler * handler;
    OctreeGeometryBuilder(InstanceBuilderHandler * handler);
    ~OctreeGeometryBuilder();

    InstanceGeometry * build(OctreeNodeData &params) override;

};

class VegetationInstanceBuilder : public OctreeNodeTriangleHandler {
	public: 
	std::vector<InstanceData> * instances;
    float pointsPerArea;
	float scale;
	
	using OctreeNodeTriangleHandler::OctreeNodeTriangleHandler;
	VegetationInstanceBuilder(long * count,std::vector<InstanceData> * instances, float pointsPerArea, float scale);
	void handle(OctreeNode* c0,OctreeNode* c1,OctreeNode* c2, bool sign) override;
};

struct NodeInfo {
	InstanceGeometry * loadable;
	DrawableInstanceGeometry * drawable;

	NodeInfo(InstanceGeometry * loadable){
		this->drawable = NULL;
		this->loadable = loadable;
	}

	~NodeInfo() {
		if(drawable != NULL) {
			delete drawable;
		}
		if(loadable != NULL) {
			delete loadable;
		}
	}
};

class Scene {
    public: 
	Octree * solidSpace;
	Octree * liquidSpace;

	long solidTrianglesCount;
	long liquidTrianglesCount;
	long solidInstancesVisible;
	long liquidInstancesVisible;
	long vegetationInstancesVisible;

	std::vector<OctreeNodeData> visibleSolidNodes;
	std::vector<OctreeNodeData> visibleLiquidNodes;
	std::vector<OctreeNodeData> visibleShadowNodes[SHADOW_MATRIX_COUNT];
	Settings * settings;

	Simplifier simplifier;
	MeshGeometryBuilder * solidBuilder;
	MeshGeometryBuilder * liquidBuilder;
	VegetationGeometryBuilder * vegetationBuilder;
	OctreeGeometryBuilder * debugBuilder;

	std::unordered_map<long, NodeInfo> solidInfo;
	std::unordered_map<long, NodeInfo> liquidInfo;
	std::unordered_map<long, NodeInfo> debugInfo;
	std::unordered_map<long, NodeInfo> vegetationInfo;

	OctreeVisibilityChecker * solidRenderer;
	OctreeVisibilityChecker * liquidRenderer;
	OctreeVisibilityChecker * shadowRenderer[SHADOW_MATRIX_COUNT];

	Scene(Settings * settings);
	DrawableInstanceGeometry * loadIfNeeded(std::unordered_map<long, NodeInfo> * infos, long index);

	bool processSpace();
	bool processLiquid(OctreeNodeData &data, Octree * tree);
	bool processSolid(OctreeNodeData &data, Octree * tree);

	void setVisibility(glm::mat4 viewProjection, std::vector<std::pair<glm::mat4, glm::vec3>> lightProjection ,Camera &camera);
	void setVisibleNodes(Octree * tree, glm::mat4 viewProjection, glm::vec3 sortPosition, OctreeVisibilityChecker &checker);

	void draw (uint drawableType, int mode, glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list);
	void drawVegetation(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list);
	void draw3dSolid(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) ;
	void draw3dLiquid(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list);
	void draw3dOctree(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list);
	void import(const std::string &filename, Camera &camera) ;
	void generate(Camera &camera) ;
	bool loadSpace(Octree * tree, OctreeNodeData &data, std::unordered_map<long, NodeInfo> *infos, GeometryBuilder * builder);
	void save(std::string folderPath, Camera &camera);
	void load(std::string folderPath, Camera &camera);
};

class DirtyHandler : public OctreeNodeDirtyHandler {
	Scene &scene;
	public:
	DirtyHandler(Scene &scene) : scene(scene){

	}

	void handle(OctreeNode * node) const {
		node->setDirty(true);
	};
};

template <typename T> class Seriallizer {
	public:

	static void serialize(std::string filename, std::vector<T> &list){
		std::ofstream file = std::ofstream(filename, std::ios::binary);
		if (!file) {
			std::cerr << "Error opening file for writing: " << filename << std::endl;
			return;
		}
	
		size_t size = list.size();
		//std::cout << std::to_string(sizeof(OctreeNodeSerialized)) << " bytes/node" << std::endl;
		std::ostringstream decompressed;
		decompressed.write(reinterpret_cast<const char*>(&size), sizeof(size_t) );
		for(size_t i =0 ; i < size ; ++i) {
			decompressed.write(reinterpret_cast<const char*>(&list.data()[i]), sizeof(T) );
		}	
		std::istringstream inputStream(decompressed.str());
			gzipCompressToOfstream(inputStream, file);
		file.close();
		std::cout << "T::serialize('" << filename <<"'," << std::to_string(size) <<") Ok!" << std::endl;
	}
	
	static void deserialize(std::string filename, std::vector<T> &list){
		std::ifstream file = std::ifstream(filename, std::ios::binary);
		if (!file) {
			std::cerr << "Error opening file for reading: " << filename << std::endl;
			return;
		}
	
		std::stringstream decompressed = gzipDecompressFromIfstream(file);
	
		size_t size;
		decompressed.read(reinterpret_cast<char*>(&size), sizeof(size_t) );
		list.resize(size);
		for(size_t i =0 ; i < size ; ++i) {
			decompressed.read(reinterpret_cast<char*>(&list.data()[i]), sizeof(T));
		}
	
		file.close();
		std::cout << "T::deserialize('" << filename <<"'," << std::to_string(size) <<") Ok!" << std::endl;
	}
};

class EnvironmentFile {
	public:
	std::string solidFilename;
	std::string liquidFilename;
	std::string brushesFilename;
	Camera * camera;
	EnvironmentFile(std::string solidFilename, std::string liquidFilename, std::string brushesFilename, Camera * camera);
	EnvironmentFile(std::string filename, Camera * camera);

	void save(std::string filename);
};

#endif
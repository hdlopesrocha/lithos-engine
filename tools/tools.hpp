#ifndef TOOLS_HPP
#define TOOLS_HPP

#define TYPE_INSTANCE_AMOUNT_DRAWABLE 0x1
#define TYPE_INSTANCE_FULL_DRAWABLE 0x2

#include "../gl/gl.hpp"
#include "../space/space.hpp"
#include "../event/event.hpp"
#include <algorithm>
#include <random>

class WaveSurface : public HeightFunction {
    float amplitude;
    float offset;
    float frequency;

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

class VegetationInstanceBuilderHandler : public InstanceBuilderHandler<InstanceData> {
	public:
	float pointsPerArea;
	float scale;
	Octree * tree;
	VegetationInstanceBuilderHandler(Octree * tree, float pointsPerArea, float scale);

	void handle(OctreeNodeData &data, std::vector<InstanceData> * instances) override;
};

class OctreeInstanceBuilderHandler : public InstanceBuilderHandler<DebugInstanceData>  {
	public:
	OctreeInstanceBuilderHandler();

	void handle(OctreeNodeData &data, std::vector<DebugInstanceData> * instances) override;
};

class VegetationGeometryBuilder : public GeometryBuilder<InstanceData> {
    public:
    Geometry * geometry;
    Octree * tree;
    InstanceBuilderHandler<InstanceData>  * handler;
	long * instancesCount;
    VegetationGeometryBuilder(Octree * tree, InstanceBuilderHandler<InstanceData>  * handler);
    ~VegetationGeometryBuilder();

	InstanceGeometry<InstanceData> * build(OctreeNodeData &params) override;
};

class OctreeGeometryBuilder : public GeometryBuilder<DebugInstanceData> {
    public:
    Geometry * geometry;
    InstanceBuilderHandler<DebugInstanceData>  * handler;
    OctreeGeometryBuilder(InstanceBuilderHandler<DebugInstanceData>  * handler);
    ~OctreeGeometryBuilder();

    InstanceGeometry<DebugInstanceData> * build(OctreeNodeData &params) override;
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

template <typename T> struct NodeInfo {
	InstanceGeometry<T> * loadable;
	DrawableInstanceGeometry<T> * drawable;

	NodeInfo(InstanceGeometry<T> * loadable){
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
	Octree * brushSpace;

	long brushTrianglesCount;
	long solidTrianglesCount;
	long liquidTrianglesCount;

	long brushInstancesVisible;
	long solidInstancesVisible;
	long liquidInstancesVisible;
	long vegetationInstancesVisible;
	long debugInstancesVisible;

	std::vector<OctreeNodeData> visibleSolidNodes;
	std::vector<OctreeNodeData> visibleBrushNodes;
	std::vector<OctreeNodeData> visibleLiquidNodes;
	std::vector<OctreeNodeData> visibleShadowNodes[SHADOW_MATRIX_COUNT];
	Settings * settings;

	Simplifier simplifier;
	MeshGeometryBuilder * brushBuilder;
	MeshGeometryBuilder * solidBuilder;
	MeshGeometryBuilder * liquidBuilder;
	VegetationGeometryBuilder * vegetationBuilder;
	OctreeGeometryBuilder * debugBuilder;

	std::unordered_map<long, NodeInfo<InstanceData>> solidInfo;
	std::unordered_map<long, NodeInfo<InstanceData>> brushInfo;
	std::unordered_map<long, NodeInfo<InstanceData>> liquidInfo;
	std::unordered_map<long, NodeInfo<DebugInstanceData>> debugInfo;
	std::unordered_map<long, NodeInfo<InstanceData>> vegetationInfo;

	OctreeVisibilityChecker * solidRenderer;
	OctreeVisibilityChecker * brushRenderer;
	OctreeVisibilityChecker * liquidRenderer;
	OctreeVisibilityChecker * shadowRenderer[SHADOW_MATRIX_COUNT];

	Scene(Settings * settings);

	bool processSpace();
	bool processLiquid(OctreeNodeData &data, Octree * tree);
	bool processSolid(OctreeNodeData &data, Octree * tree);

	void setVisibility(glm::mat4 viewProjection, std::vector<std::pair<glm::mat4, glm::vec3>> lightProjection ,Camera &camera);
	void setVisibleNodes(Octree * tree, glm::mat4 viewProjection, glm::vec3 sortPosition, OctreeVisibilityChecker &checker);

	template <typename T, typename H> void draw (uint drawableType, int mode, glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list, std::unordered_map<long, NodeInfo<T>> * info,long * count);
	void drawVegetation(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list);
	void draw3dSolid(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) ;
	void draw3dLiquid(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list);
	void draw3dOctree(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list);
	void draw3dBrush(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list);

	void import(const std::string &filename, Camera &camera) ;
	void generate(Camera &camera) ;
	template <typename T> bool loadSpace(Octree * tree, OctreeNodeData &data, std::unordered_map<long, NodeInfo<T>> *infos, GeometryBuilder<T> * builder);
	template <typename T> DrawableInstanceGeometry<T> * loadIfNeeded(std::unordered_map<long, NodeInfo<T>> * infos, long index, InstanceHandler<T> * handler);

	void save(std::string folderPath, Camera &camera);
	void load(std::string folderPath, Camera &camera);
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

class OctreeDifferenceFunction : public SignedDistanceFunction {
    Octree * tree;
    BoundingBox box;
    public:
    OctreeDifferenceFunction(Octree * tree, BoundingBox box);
    float distance(const glm::vec3 p) const override;
	SdfType getType() const override;
};


#define TYPE_FLOAT 1
#define TYPE_VEC2 2
#define TYPE_VEC3 3

class ControlledAttributeBase {
	public:
	virtual int getType() = 0;
};

template <typename T> class ControlledAttribute : public ControlledAttributeBase {
    T * value;
    int type;
    public:
    
    ControlledAttribute(T * value, int type) {
        value = value;
        type = type;
    }

    int getType() override {
        return type;
    }

    T * getValue() {
        return value;
    }
};

class ControlledObject {
    public:
    std::vector<ControlledAttributeBase*> attributes;
};

class BrushContext {
	public:
	std::vector<SignedDistanceFunction*> functions;
	SignedDistanceFunction * currentFunction;
	BoundingSphere boundingVolume;
	Simplifier * simplifier;
	float detail;
	Camera &camera;

	BrushContext(Camera &camera);
	void handleEvent(Event &event);
};

template<typename T> class TimedAttribute {
    public:
    float deltaTime;
    T value;
    TimedAttribute(float deltaTime, T value) : deltaTime(deltaTime), value(value) {}

};


#endif
#ifndef TOOLS_HPP
#define TOOLS_HPP

#define TYPE_INSTANCE_AMOUNT_DRAWABLE 0x1
#define TYPE_INSTANCE_FULL_DRAWABLE 0x2

#include "../gl/gl.hpp"
#include "../space/space.hpp"
#include "../handler/handler.hpp"

#include <algorithm>
#include <random>

enum Tab {
	PAGE_ROTATION,
	PAGE_SCALE,
	PAGE_TRANSLATE,
	PAGE_SDF0,
	PAGE_SDF1,
	PAGE_SDF2,
	COUNT
};

class ComputeShader {
	GLuint program;
	GLuint vertexSSBO;
	GLuint indexSSBO;
	GLuint counterSSBO;
    GLuint octreeSSBO;
    GLuint nodesSSBO;
	
	public:
	ComputeShader(GLuint program);
	void allocateSSBO();
	void dispatch();
	void writeSSBO(OctreeSerialized * octree, std::vector<OctreeNodeSerialized> * nodes);
};


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
	bool update;

	InstanceGeometry<T> * loadable;
	DrawableInstanceGeometry<T> * drawable;

	NodeInfo(InstanceGeometry<T> * loadable){
		this->update = false;
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


class LiquidSpaceChangeHandler : public OctreeChangeHandler {
	std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> * liquidInfo;
 
	public:
	LiquidSpaceChangeHandler(
		std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> * liquidInfo
	);

	void create(OctreeNode* nodeId) override;
	void update(OctreeNode* nodeId) override;
	void erase(OctreeNode* nodeId) override;
};

class SolidSpaceChangeHandler : public OctreeChangeHandler {
	std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> * solidInfo;
	std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> * vegetationInfo;
	std::unordered_map<OctreeNode*, NodeInfo<DebugInstanceData>> * debugInfo;
 
	public:
	SolidSpaceChangeHandler(
		std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> * solidInfo,
		std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> * vegetationInfo,
		std::unordered_map<OctreeNode*, NodeInfo<DebugInstanceData>> * debugInfo
	);

	void create(OctreeNode* nodeId) override;
	void update(OctreeNode* nodeId) override;
	void erase(OctreeNode* nodeId) override;
};

class BrushSpaceChangeHandler : public OctreeChangeHandler {
	std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> * brushInfo;
 
	public:
	BrushSpaceChangeHandler(
		std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> * brushInfo
	);

	void create(OctreeNode* nodeId) override;
	void update(OctreeNode* nodeId) override;
	void erase(OctreeNode* nodeId) override;
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
	
	Simplifier simplifier;
	MeshGeometryBuilder * brushBuilder;
	MeshGeometryBuilder * solidBuilder;
	MeshGeometryBuilder * liquidBuilder;
	VegetationGeometryBuilder * vegetationBuilder;
	OctreeGeometryBuilder * debugBuilder;

	std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> solidInfo;
	std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> brushInfo;
	std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> liquidInfo;
	std::unordered_map<OctreeNode*, NodeInfo<DebugInstanceData>> debugInfo;
	std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> vegetationInfo;

	LiquidSpaceChangeHandler * liquidSpaceChangeHandler;
	SolidSpaceChangeHandler * solidSpaceChangeHandler;
	BrushSpaceChangeHandler * brushSpaceChangeHandler;

	OctreeVisibilityChecker * solidRenderer;
	OctreeVisibilityChecker * brushRenderer;
	OctreeVisibilityChecker * liquidRenderer;
	OctreeVisibilityChecker * shadowRenderer[SHADOW_MATRIX_COUNT];

	Settings * settings;
	ComputeShader &computeShader;
	

	Scene(Settings * settings, ComputeShader &computeShader);

	bool processSpace();
	bool processLiquid(OctreeNodeData &data, Octree * tree);
	bool processSolid(OctreeNodeData &data, Octree * tree);
	bool processBrush(OctreeNodeData &data, Octree * tree);

	void setVisibility(glm::mat4 viewProjection, std::vector<std::pair<glm::mat4, glm::vec3>> lightProjection ,Camera &camera);
	void setVisibleNodes(Octree * tree, glm::mat4 viewProjection, glm::vec3 sortPosition, OctreeVisibilityChecker &checker);

	template <typename T, typename H> void draw (uint drawableType, int mode, glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list, std::unordered_map<OctreeNode*, NodeInfo<T>> * info,long * count);
	void drawVegetation(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list);
	void draw3dSolid(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) ;
	void draw3dLiquid(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list);
	void draw3dOctree(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list);
	void draw3dBrush(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list);

	void import(const std::string &filename, Camera &camera) ;
	void generate(Camera &camera) ;
	template <typename T> bool loadSpace(Octree * tree, OctreeNodeData &data, std::unordered_map<OctreeNode*, NodeInfo<T>> *infos, GeometryBuilder<T> * builder);
	template <typename T> DrawableInstanceGeometry<T> * loadIfNeeded(std::unordered_map<OctreeNode*, NodeInfo<T>> * infos, OctreeNode* node, InstanceHandler<T> * handler);

	void save(std::string folderPath, Camera &camera);
	void load(std::string folderPath, Camera &camera);


};

class BrushContext {
	public:
	BrushMode mode;
	std::vector<SignedDistanceFunction*> functions;
	SignedDistanceFunction * currentFunction;
	BoundingSphere boundingVolume;
	Simplifier * simplifier;
	Tab currentTab;

	float detail;
	Camera * camera;
	int brushIndex;
	Scene &scene;
	Transformation model;

	BrushContext(Camera *camera, Scene &scene);
	void apply(Octree &space, OctreeChangeHandler &handler, bool preview);
	WrappedSignedDistanceFunction * getWrapped();
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
    public:
    Octree * tree;
    BoundingBox box;
    OctreeDifferenceFunction(Octree * tree, BoundingBox box);
    float distance(const glm::vec3 p, Transformation model) const override;
	SdfType getType() const override;
	glm::vec3 getCenter(Transformation model) const override;

};

class WrappedOctreeDifference : public WrappedSignedDistanceFunction {
    public:
    WrappedOctreeDifference(OctreeDifferenceFunction * function, float bias, Transformation model) : WrappedSignedDistanceFunction(function, bias, model) {

    }

    BoundingBox getBox() const {
        OctreeDifferenceFunction * f = (OctreeDifferenceFunction*) function;
        return BoundingBox(f->box.getMin()-glm::vec3(bias), f->box.getMax()+glm::vec3(bias));
    }
        
    ContainmentType check(const BoundingCube &cube) const override {
        BoundingBox box = getBox();
        return box.test(cube);
    };

    bool isContained(const BoundingCube &cube) const override {
        BoundingBox box = getBox();
        return cube.contains(box);
    };

    glm::vec3 getCenter(Transformation model) const override {
        BoundingBox box = getBox();
        return box.getCenter();
    };
};



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


template<typename T> class TimedAttribute {
    public:
    float deltaTime;
    T value;
    TimedAttribute(float deltaTime, T value) : deltaTime(deltaTime), value(value) {}

};


class PaintBrushHandler : public EventHandler<Event>{
    Brush3d * brush3d;
    Octree * octree;
    Simplifier simplifier;
    public:
    PaintBrushHandler(Brush3d * brush3d, Octree * octree);
    void handle(Event * event) override ;
};

class CloseWindowHandler : public EventHandler<Event>{
    LithosApplication *app;
    public:
    CloseWindowHandler(LithosApplication *app);
    void handle(Event * value) override ;
};

template<typename T> class BrushEventHandler : public EventHandler<T> {
	BrushContext &context;
	Scene &scene;

	public:
	BrushEventHandler(BrushContext &context, Scene &scene);

	void handle(T * event) override;
};

template class BrushEventHandler<Event>;
template class BrushEventHandler<Axis3dEvent>;
template class BrushEventHandler<FloatEvent>;



#endif
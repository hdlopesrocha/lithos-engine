#ifndef TOOLS_HPP
#define TOOLS_HPP

#define NDEBUG 1
#define DEBUG_OCTREE_WIREFRAME 1
#define STARTUP_GENERATE 1
//#define CLOSE_AFTER_GENERATE 1
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

class OctreeInstanceBuilderHandler : public InstanceBuilderHandler<DebugInstanceData>  {
	public:
	OctreeInstanceBuilderHandler();

	void handle(Octree * tree, OctreeNodeData &data, std::vector<DebugInstanceData> * instances, ThreadContext * context) override;
};


class OctreeGeometryBuilder : public GeometryBuilder<DebugInstanceData> {
    public:
    Geometry * geometry;
    InstanceBuilderHandler<DebugInstanceData>  * handler;
    OctreeGeometryBuilder(InstanceBuilderHandler<DebugInstanceData>  * handler);
    ~OctreeGeometryBuilder();

    InstanceGeometry<DebugInstanceData> * build(Octree * tree, OctreeNodeData &params, ThreadContext * context) override;
};

class VegetationInstanceBuilder : public OctreeNodeTriangleHandler {
	public: 
	std::vector<InstanceData> * instances;
    float pointsPerArea;
	float scale;
	
	using OctreeNodeTriangleHandler::OctreeNodeTriangleHandler;
	VegetationInstanceBuilder(Octree * tree, long * count,std::vector<InstanceData> * instances, float pointsPerArea, float scale);
	void handle(OctreeNodeData &data, Vertex &v0, Vertex &v1, Vertex &v2, bool signn) override;
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

template <typename T> struct OctreeLayer {
	std::unordered_map<OctreeNode*, NodeInfo<T>> info;
    std::shared_mutex mutex;
    std::shared_mutex mutex2;
};


class LiquidSpaceChangeHandler : public OctreeChangeHandler {
	OctreeLayer<InstanceData> * liquidInfo;

	public:
	LiquidSpaceChangeHandler(
		OctreeLayer<InstanceData> * liquidInfo
	);

	void create(OctreeNode* nodeId) override;
	void update(OctreeNode* nodeId) override;
	void erase(OctreeNode* nodeId) override;
};

class SolidSpaceChangeHandler : public OctreeChangeHandler {
	OctreeLayer<InstanceData> * vegetationInfo;

	public:
	SolidSpaceChangeHandler(
		OctreeLayer<InstanceData> * vegetationInfo
	);

	void create(OctreeNode* nodeId) override;
	void update(OctreeNode* nodeId) override;
	void erase(OctreeNode* nodeId) override;
};

class BrushSpaceChangeHandler : public OctreeChangeHandler {
	OctreeLayer<InstanceData> * brushInfo;
	OctreeLayer<DebugInstanceData> * octreeWireframeInfo;
	public:
	BrushSpaceChangeHandler(
		OctreeLayer<InstanceData> * brushInfo,
		OctreeLayer<DebugInstanceData> * octreeWireframeInfo
	);

	void create(OctreeNode* nodeId) override;
	void update(OctreeNode* nodeId) override;
	void erase(OctreeNode* nodeId) override;
};

class BrushContext {
	public:
	BrushMode mode;
	std::vector<WrappedSignedDistanceFunction*> functions;
	WrappedSignedDistanceFunction * currentFunction;
	std::vector<WrappedSignedDistanceEffect*> effects;
	WrappedSignedDistanceEffect * currentEffect;
	Simplifier * simplifier;
	Tab currentTab;

	float detail;
	int brushIndex;
	Settings * settings;
	Camera * camera;	
	Transformation model;

	BrushContext(Settings * settings, Camera * camera);
	void apply(Octree &space, OctreeChangeHandler * handler, bool preview);
};



class Scene {
    public: 
	Octree solidSpace;
	Octree liquidSpace;
	Octree brushSpace;

	long brushTrianglesCount;
	long trianglesCount;

	long brushInstancesVisible;
	long solidInstancesVisible;
	long liquidInstancesVisible;
	long vegetationInstancesVisible;
	long debugInstancesVisible;

	OctreeGeometryBuilder * debugBuilder;

	OctreeLayer<InstanceData> brushInfo;
	OctreeLayer<InstanceData> liquidInfo;
	OctreeLayer<InstanceData> solidInfo;
	OctreeLayer<DebugInstanceData> octreeWireframeInfo;
	OctreeLayer<InstanceData> vegetationInfo;

	LiquidSpaceChangeHandler * liquidSpaceChangeHandler;
	SolidSpaceChangeHandler * solidSpaceChangeHandler;
	BrushSpaceChangeHandler * brushSpaceChangeHandler;
	
	OctreeVisibilityChecker * solidRenderer;
	OctreeVisibilityChecker * brushRenderer;
	OctreeVisibilityChecker * liquidRenderer;
	OctreeVisibilityChecker * shadowRenderer[SHADOW_MATRIX_COUNT];

	Settings * settings;
	BrushContext * brushContext;
	Vegetation3d * vegetationGeometry;
	ThreadPool threadPool = ThreadPool(std::thread::hardware_concurrency());


	Scene(Settings * settings, BrushContext * brushContext);

	bool processSpace();
	bool processLiquid(OctreeNodeData &data, Octree * tree);
	bool processSolid(OctreeNodeData &data, Octree * tree);
	bool processBrush(OctreeNodeData &data, Octree * tree);

	void setVisibility(glm::mat4 viewProjection, std::vector<std::pair<glm::mat4, glm::vec3>> lightProjection ,Camera &camera);
	void setVisibleNodes(Octree * tree, glm::mat4 viewProjection, glm::vec3 sortPosition, OctreeVisibilityChecker * checker);

	template <typename T, typename H> void draw (uint drawableType, int mode, glm::vec3 cameraPosition, const OctreeVisibilityChecker * checker, OctreeLayer<T> * info,long * count);
	void drawVegetation(glm::vec3 cameraPosition, const OctreeVisibilityChecker * checker);
	void draw3dSolid(glm::vec3 cameraPosition, const OctreeVisibilityChecker * checker) ;
	void draw3dLiquid(glm::vec3 cameraPosition, const OctreeVisibilityChecker * checker);
	void draw3dOctree(glm::vec3 cameraPosition, const OctreeVisibilityChecker * checker);
	void draw3dBrush(glm::vec3 cameraPosition, const OctreeVisibilityChecker * checker);

	void import(const std::string &filename, Camera &camera) ;
	void generate(Camera &camera) ;
	template <typename T> bool loadSpace(Octree * tree, OctreeNodeData &data, OctreeLayer<T> *infos, InstanceGeometry<T>* loadable);
	template <typename T> DrawableInstanceGeometry<T> * loadIfNeeded(OctreeLayer<T> * infos, OctreeNode* node, InstanceHandler<T> * handler);

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
    public:
    Octree * tree;
    BoundingBox box;
	float bias;
    OctreeDifferenceFunction(Octree * tree, BoundingBox box, float bias);
    float distance(const glm::vec3 p, const Transformation &model) override;
	SdfType getType() const override;
	glm::vec3 getCenter(const Transformation &model) const override;

};

class WrappedOctreeDifference : public WrappedSignedDistanceFunction {
    public:
    WrappedOctreeDifference(OctreeDifferenceFunction * function) : WrappedSignedDistanceFunction(function) {

    }

    ~WrappedOctreeDifference() {

    }

    BoundingBox getBox(float bias) const {
        OctreeDifferenceFunction * f = (OctreeDifferenceFunction*) function;
        return BoundingBox(f->box.getMin()-glm::vec3(bias), f->box.getMax()+glm::vec3(bias));
    }
        
    ContainmentType check(const BoundingCube &cube, const Transformation &model, float bias) const override {
        BoundingBox box = getBox(bias);
        return box.test(cube);
    };

    bool isContained(const BoundingCube &cube, const Transformation &model, float bias) const override {
        BoundingBox box = getBox(bias);
        return cube.contains(box);
    };

    glm::vec3 getCenter(const Transformation &model) const override {
        OctreeDifferenceFunction * f = (OctreeDifferenceFunction*) function;
        return f->box.getCenter();
    };
	float getLength(const Transformation &model, float bias) const override {
		BoundingBox box = getBox(bias);
		return glm::length(box.getLength()) + bias;
	};

    void accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const override {
        getBox(bias).accept(visitor);
    }

	const char* getLabel() const override {
    	return "Octree Difference";
	}
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
#ifndef TOOLS_HPP
#define TOOLS_HPP
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
	glm::vec3 getCenter() const override;
	bool contains(const glm::vec3 p) const override;
	bool isContained(const BoundingCube &p) const override;
	ContainmentType check(const BoundingCube &cube) const override;
	Vertex getVertex(const BoundingCube &cube, ContainmentType solid, glm::vec3 previousPoint) const override;
};

class VegetationInstanceBuilderHandler : public InstanceBuilderHandler {
	public:
	int pointsPerTriangle;
	float scale;

	VegetationInstanceBuilderHandler(Octree * tree, long * count, int pointsPerTriangle, float scale);

	void handle(OctreeNodeData &data, InstanceGeometry * pre) override;
};


class OctreeInstanceBuilderHandler : public InstanceBuilderHandler {
	public:

	OctreeInstanceBuilderHandler(Octree * tree, long * count);

	void handle(OctreeNodeData &data, InstanceGeometry * pre) override;
};


class VegetationGeometryBuilder : public GeometryBuilder {
    public:
    Geometry * geometry;
    Octree * tree;
    InstanceBuilderHandler * handler;
	long * instancesCount;
    VegetationGeometryBuilder(long * instancesCount, Octree * tree, InstanceBuilderHandler * handler);
    ~VegetationGeometryBuilder();

	InstanceGeometry * build(OctreeNodeData &params) override;
};

class OctreeGeometryBuilder : public GeometryBuilder {
    public:
    Geometry * geometry;
    Octree * tree;
	long * instancesCount;
    InstanceBuilderHandler * handler;
    OctreeGeometryBuilder(long * instancesCount, Octree * tree, InstanceBuilderHandler * handler);
    ~OctreeGeometryBuilder();

    InstanceGeometry * build(OctreeNodeData &params) override;

};

class VegetationInstanceBuilder : public OctreeNodeTriangleHandler {

	public: 
	std::vector<InstanceData> * instances;
    int pointsPerTriangle;
	float scale;
	
	using OctreeNodeTriangleHandler::OctreeNodeTriangleHandler;
	VegetationInstanceBuilder(Geometry * chunk, long * count,std::vector<InstanceData> * instances, int pointsPerTriangle, float scale);
	void handle(OctreeNode* c0,OctreeNode* c1,OctreeNode* c2, bool sign) override;

};

struct NodeInfo {
	InstanceGeometry * loadable;
	DrawableInstanceGeometry * drawable;
	bool dirty;

	NodeInfo(InstanceGeometry * loadable){
		this->drawable = NULL;
		this->loadable = loadable;
		this->dirty = false;
	}
};


class Scene {

    public: 

    	Octree * solidSpace;
	    Octree * liquidSpace;

		long solidTrianglesCount;
		long liquidTrianglesCount;
		long solidInstancesCount;
		long liquidInstancesCount;
		long vegetationInstancesCount;
		long octreeInstancesCount;

		long solidInstancesVisible;
		long liquidInstancesVisible;
		long vegetationInstancesVisible;

		std::vector<OctreeNodeData> visibleSolidNodes;
		std::vector<OctreeNodeData> visibleLiquidNodes;
		std::vector<OctreeNodeData> visibleShadowNodes[SHADOW_MATRIX_COUNT];
		Settings * settings;
		float chunkSize;

		OctreeProcessor * solidProcessor; 
		OctreeProcessor * liquidProcessor;

MeshGeometryBuilder * solidBuilder;
MeshGeometryBuilder * liquidBuilder;
VegetationGeometryBuilder * vegetationBuilder;
OctreeGeometryBuilder * debugBuilder;

std::unordered_map<long, NodeInfo*> solidInfo;
std::unordered_map<long, NodeInfo*> liquidInfo;
std::unordered_map<long, NodeInfo*> debugInfo;
std::unordered_map<long, NodeInfo*> vegetationInfo;


		OctreeVisibilityChecker * solidRenderer;
		OctreeVisibilityChecker * liquidRenderer;
		OctreeVisibilityChecker * shadowRenderer[SHADOW_MATRIX_COUNT];

	Scene(Settings * settings);
	DrawableInstanceGeometry * loadIfNeeded(std::unordered_map<long, NodeInfo*> * infos, long index);




	void processSpace();

	void setVisibility(glm::mat4 viewProjection, std::vector<std::pair<glm::mat4, glm::vec3>> lightProjection ,Camera &camera);

	void setVisibleNodes(glm::mat4 viewProjection, glm::vec3 sortPosition, OctreeVisibilityChecker &checker);

	void draw (uint drawableType, int mode, glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list);
	void drawVegetation(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list);
	void draw3dSolid(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) ;
	void draw3dLiquid(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list);
	void draw3dOctree(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list);
	void import(const std::string &filename, Camera &camera) ;
	void generate(Camera &camera) ;

	void save(std::string folderPath, Camera &camera);
	void load(std::string folderPath, Camera &camera);
};

class DirtyHandler : public OctreeNodeDirtyHandler {

	Scene &scene;

	public:
	DirtyHandler(Scene &scene) : scene(scene){

	}

	void singleHandle(std::unordered_map<long, NodeInfo*> * infos, long dataId) const {
		auto i = infos->find(dataId);
		NodeInfo * ni = i != infos->end() ? i->second : NULL;
		if(ni!=NULL) {
			ni->dirty = true;
		}
	}


	void handle(long dataId) const {
		singleHandle(&scene.solidInfo, dataId);
		singleHandle(&scene.vegetationInfo, dataId);
		singleHandle(&scene.liquidInfo, dataId);
		singleHandle(&scene.debugInfo, dataId);
	};

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
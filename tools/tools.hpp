#ifndef TOOLS_HPP
#define TOOLS_HPP
#include "../gl/gl.hpp"

#define DISCARD_BRUSH_INDEX -1

class WaveSurface : public HeightFunction {
	float getHeightAt(float x, float z) {
		float amplitude = 10;
		float offset = -36;
		float frequency = 1.0/10.0;

		return offset + amplitude * sin(frequency*x)*cos(frequency*z);
	}
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

	VegetationInstanceBuilderHandler(Octree * tree, long * count);

	void handle(OctreeNode *node, const BoundingCube &cube, int level, InstanceGeometry * pre) override;
};

class VegetationGeometryBuilder : public GeometryBuilder {
    public:
    Geometry * geometry;
    Octree * tree;
    InstanceBuilderHandler * handler;
    VegetationGeometryBuilder(int drawableType, long * count, Octree * tree, InstanceBuilderHandler * handler);
    ~VegetationGeometryBuilder();

    const NodeInfo build(int level, int height, int lod, OctreeNode* node, BoundingCube cube) override;

};


class Scene {

    public: 
    	Octree * solidSpace = new Octree(BoundingCube(glm::vec3(0,0,0), 2.0));
	    Octree * liquidSpace = new Octree(BoundingCube(glm::vec3(0,20,0), 2.0));

		long solidInstancesCount = 0;
		long liquidInstancesCount = 0;
		long vegetationInstancesCount = 0;

		long solidInstancesVisible = 0;
		long liquidInstancesVisible = 0;
		long vegetationInstancesVisible = 0;

		std::vector<IteratorData> visibleSolidNodes;
		std::vector<IteratorData> visibleLiquidNodes;
		std::vector<IteratorData> visibleShadowNodes[SHADOW_MATRIX_COUNT];
		Settings * settings;
		int geometryLevel = 5;

		InstanceBuilderHandler * vegetationInstanceHandler= new VegetationInstanceBuilderHandler(solidSpace, &vegetationInstancesCount);


		GeometryBuilder * vegetationBuilder = new VegetationGeometryBuilder(TYPE_INSTANCE_VEGETATION_DRAWABLE, &vegetationInstancesCount, solidSpace, vegetationInstanceHandler);
		GeometryBuilder * meshBuilder = new MeshGeometryBuilder(TYPE_INSTANCE_SOLID_DRAWABLE, &solidInstancesCount, solidSpace, 0.9, 0.2, true, geometryLevel);
		GeometryBuilder * liquidMeshBuilder = new MeshGeometryBuilder(TYPE_INSTANCE_LIQUID_DRAWABLE, &liquidInstancesCount, liquidSpace, 0.9, 0.2, true, geometryLevel);

		OctreeProcessor solidProcessor = OctreeProcessor(solidSpace , true, meshBuilder);
		OctreeProcessor liquidProcessor = OctreeProcessor(liquidSpace, true, liquidMeshBuilder);
		OctreeProcessor vegetationProcessor = OctreeProcessor(solidSpace, true, vegetationBuilder);

		OctreeVisibilityChecker solidRenderer = OctreeVisibilityChecker(solidSpace, &visibleSolidNodes);
		OctreeVisibilityChecker liquidRenderer = OctreeVisibilityChecker(liquidSpace, &visibleLiquidNodes);
		OctreeVisibilityChecker shadowRenderer[SHADOW_MATRIX_COUNT] = {
			OctreeVisibilityChecker(solidSpace, &visibleShadowNodes[0]),
			OctreeVisibilityChecker(solidSpace, &visibleShadowNodes[1]),
			OctreeVisibilityChecker(solidSpace, &visibleShadowNodes[2])
		};

    void setup(Settings * settings);



	void processSpace();

	void setVisibility(glm::mat4 viewProjection, std::vector<std::pair<glm::mat4, glm::vec3>> lightProjection ,Camera &camera);

	void setVisibleNodes(glm::mat4 viewProjection, glm::vec3 sortPosition, OctreeVisibilityChecker &checker);

	void draw (uint drawableType, int mode, glm::vec3 cameraPosition, const std::vector<IteratorData> &list);
	void drawVegetation(glm::vec3 cameraPosition, const std::vector<IteratorData> &list);
	void draw3dSolid(glm::vec3 cameraPosition, const std::vector<IteratorData> &list) ;
	void draw3dLiquid(glm::vec3 cameraPosition, const std::vector<IteratorData> &list);
	void create() ;

	void save();

	void load() ;
};

#endif
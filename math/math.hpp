#ifndef MATH_HPP
#define MATH_HPP
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/integer.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp> 
#include <glm/matrix.hpp>
#include <bitset>
#include <format>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <stack>
#include <map>
#include <filesystem>
#include <algorithm>
#define DB_PERLIN_IMPL
#include "perlin.hpp"
#define INFO_TYPE_FILE 99
#define INFO_TYPE_REMOVE 0


class BoundingSphere;
class BoundingBox;
class IteratorHandler;

enum ContainmentType {
	Contains,
	Intersects,
	Disjoint
};

struct Plane {
	glm::vec3 normal;
	float d;
	public:
	Plane(glm::vec3 normal, glm::vec3 point);
	float distance(glm::vec3 &point);
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    int brushIndex;

    Vertex(){
		this->position = glm::vec3(0);
    	this->normal = glm::vec3(0);
    	this->texCoord = glm::vec2(0);
    	this->brushIndex = 0;
    }

    Vertex(glm::vec3 pos) : Vertex(){
    	this->position = pos;
    }

    Vertex(glm::vec3 pos,glm::vec3 normal,glm::vec2 texCoord, uint texIndex): Vertex() {
    	this->position = pos;
    	this->normal = normal;
    	this->texCoord = texCoord;
    	this->brushIndex = texIndex;
    }

	bool operator<(const Vertex& other) const {
        return std::tie(position.x, position.y, position.z, normal.x, normal.y, normal.z, texCoord.x, texCoord.y, brushIndex) 
             < std::tie(other.position.x, other.position.y, other.position.z, other.normal.x, other.normal.y, other.normal.z, other.texCoord.x, other.texCoord.y, other.brushIndex);
    }
};

struct Triangle {
	public:
		Vertex v[3];

		Triangle(Vertex v1, Vertex v2, Vertex v3){
			this->v[0] = v1;
			this->v[1] = v2;
			this->v[2] = v3;
		}

		Triangle flip(){
			Vertex t = this->v[1];
			this->v[1]= this->v[2];
			this->v[2]= t;
			return *this;
		}
};

enum SpaceType {
    Empty,
    Surface,
    Solid
};

class AbstractBoundingBox {
	private: 
	glm::vec3 min;

	public:
	AbstractBoundingBox();
	AbstractBoundingBox(glm::vec3 min);
    virtual ~AbstractBoundingBox() {}  

	float getMinX() const;
	float getMinY() const;
	float getMinZ() const;
	glm::vec3 getMin() const;
	void setMin(glm::vec3 v);
	glm::vec3 getCenter() const;

	virtual	float getMaxX() const = 0;
	virtual	float getMaxY() const = 0;
	virtual	float getMaxZ() const = 0;
	virtual	glm::vec3 getMax() const = 0;	

	virtual float getLengthX() const =0;
	virtual float getLengthY() const =0;
	virtual float getLengthZ() const =0;
	virtual glm::vec3 getLength() const =0;


	bool contains(const glm::vec3 &point) const;
	bool contains(const BoundingSphere &sphere) const;
	bool contains(const AbstractBoundingBox &cube) const;
	bool intersects(const BoundingSphere &sphere) const;
	ContainmentType test(const AbstractBoundingBox &cube) const;
};

class BoundingCube : public AbstractBoundingBox {
	private: 
		float length;
	
	public: 
		BoundingCube();
		BoundingCube(glm::vec3 min, float length);
		void setLength(float l);

		glm::vec3 getLength() const override;
		float getLengthX() const override;
		float getLengthY() const override;
		float getLengthZ() const override;
		float getMaxX() const override;
		float getMaxY() const override;
		float getMaxZ() const override;
		glm::vec3 getMax() const override;	
};


class BoundingSphere {
	public: 
		glm::vec3 center;
		float radius;
		BoundingSphere();		
		BoundingSphere(glm::vec3 center, float radius);
		bool contains(const glm::vec3 point) const ;
		ContainmentType test(const AbstractBoundingBox& cube) const;
		bool intersects(const AbstractBoundingBox& cube) const;
};


class BoundingBox : public AbstractBoundingBox {
	private: 
		glm::vec3 max;

	public: 
		BoundingBox(glm::vec3 min, glm::vec3 max);
		BoundingBox();
		void setMax(glm::vec3 v);

		glm::vec3 getLength() const override;
		float getLengthX() const override;
		float getLengthY() const override;
		float getLengthZ() const override;
		float getMaxX() const override;
		float getMaxY() const override;
		float getMaxZ() const override;
		glm::vec3 getMax() const override;	
};


class HeightFunction {
	public:
	    virtual ~HeightFunction() {}  
		virtual float getHeightAt(float x, float y, float z) const = 0;
		glm::vec3 getNormal(float x, float z, float delta) const;

};


class CachedHeightMapSurface : public HeightFunction {
	public:
		std::vector<std::vector<float>> data; 
		BoundingBox box;
		float delta;
		int width;
		int height;


	CachedHeightMapSurface(const HeightFunction &function, BoundingBox box, float delta);
	float getData(int x, int z) const;
	float getHeightAt(float x, float yy, float z) const override;

};

class PerlinSurface : public HeightFunction {
	public:
	float amplitude;
	float frequency;
	float offset;


	PerlinSurface(float amplitude, float frequency, float offset);
	float getHeightAt(float x, float y, float z) const override;
};

class FractalPerlinSurface : public PerlinSurface {
	public:
	using PerlinSurface::PerlinSurface;
	FractalPerlinSurface(float amplitude, float frequency, float offset);
	float getHeightAt(float x, float y, float z) const override;
};


class GradientPerlinSurface : public PerlinSurface {
	public:

	GradientPerlinSurface(float amplitude, float frequency, float offset);
	float getHeightAt(float x, float y, float z) const override;
};

class HeightMap: public BoundingBox  {
	private: 
		float step;
		const HeightFunction &func;
	public: 
		HeightMap(const HeightFunction &func, BoundingBox box, float step);
		
   		glm::vec3 getNormalAt(float x, float z) const ;
		glm::vec3 getPoint(const BoundingCube& cube) const ;

		glm::ivec2 getIndexes(float x, float z) const ;
		glm::vec2 getHeightRangeBetween(const BoundingCube &cube) const ;
		bool hitsBoundary(const BoundingCube &cube) const ;
		bool isContained(const BoundingCube &p) const ;
		bool contains(const glm::vec3 &point) const ;
		ContainmentType test(const BoundingCube &cube) const ;

};

class ContainmentHandler {
	public: 
		virtual ContainmentType check(const BoundingCube &cube) const = 0;
		virtual Vertex getVertex(const BoundingCube &cube, ContainmentType solid, glm::vec3 previousPoint) const = 0;
		virtual glm::vec3 getCenter() const = 0;
		virtual bool contains(const glm::vec3 p) const = 0;
		virtual bool isContained(const BoundingCube &cube) const = 0;
};

struct NodeInfo {
	public:
	void * data;
	void * temp;
	uint type;
	bool dirty = true;

	NodeInfo(uint type, void * data, void * temp, bool dirty) {
		this->type = type;
		this->data = data;
		this->temp = temp;
		this->dirty = dirty;
	}
};

class OctreeNode {
	public: 
		Vertex vertex;
		OctreeNode *children[8];
		int simplification;
		uint mask;
		ContainmentType solid;
		std::vector<NodeInfo> info;
		OctreeNode(Vertex vertex);
		~OctreeNode();
		void clear();
		bool isEmpty();
		void setChild(int i, OctreeNode * node);
		NodeInfo * getNodeInfo(uint infoType);
};

class GeometryBuilder {
    public:
    int infoType;
    long * count;

    GeometryBuilder(int infoType,long * count);
    virtual const NodeInfo build(int level, int height, int lod, OctreeNode* node, BoundingCube cube) = 0;
};


class Geometry
{
public:
	std::vector<Vertex> vertices;
	std::vector<uint> indices;
	std::map <Vertex, int> compactMap;
	glm::vec3 center;
	Geometry(/* args */);
	~Geometry();

	Vertex * addVertex(Vertex vertex);
	static glm::vec3 getNormal(Vertex * a, Vertex * b, Vertex * c);
	glm::vec3 getCenter();
	void setCenter();

};

class OctreeNodeTriangleHandler {

	public: 
	Geometry * chunk;
	long * count;
	OctreeNodeTriangleHandler(Geometry * chunk, long * count);
	virtual void handle(OctreeNode* c0,OctreeNode* c1,OctreeNode* c2, bool sign) = 0;
};

class Octree: public BoundingCube {
	public: 
		float minSize;
		OctreeNode * root;

		Octree(BoundingCube minCube);
		void expand(const ContainmentHandler &handler);
		void add(const ContainmentHandler &handler);
		void del(const ContainmentHandler &handler);
		void iterate(IteratorHandler &handler, int geometryLevel);
		void iterateFlat(IteratorHandler &handler, int geometryLevel);

		OctreeNode* getNodeAt(const glm::vec3 &pos, int level, int simplification);
		void handleQuadNodes(const BoundingCube &cube, int level,OctreeNode &node, OctreeNodeTriangleHandler * handler);
		void getNodeNeighbors(const BoundingCube &cube, int level, int simplification, int direction, OctreeNode ** out, int initialIndex, int finalIndex);
		ContainmentType contains(const glm::vec3 &pos);
		ContainmentType contains(const AbstractBoundingBox&cube);

		static glm::vec3 getShift(int i);
		static glm::vec3 getShift3(int i);
		static BoundingCube getChildCube(const BoundingCube &cube, int i);
		static BoundingCube getCube3(const BoundingCube &cube, int i);

		int getHeight(const BoundingCube  &cube);

};

class MeshGeometryBuilder  : public GeometryBuilder {
    public:
    float simplificationAngle;
    float simplificationDistance;
    bool simplificationTexturing;
    int simplification;
    Octree * tree;
    MeshGeometryBuilder(int drawableType, long * count,Octree * tree, float simplificationAngle, float simplificationDistance, bool simplificationTexturing, int simplification);
    ~MeshGeometryBuilder();

    const NodeInfo build(int level, int height, int lod, OctreeNode* node, BoundingCube cube) override;

};


struct IteratorData {
	public:
	int level;
	int height;
	int lod;
	OctreeNode * node;
	BoundingCube cube;
	void * context;
	IteratorData(int level, int height, int lod, OctreeNode * node, BoundingCube cube, void * context) {
		this->level = level;
		this->height = height;
		this->lod = lod;
		this->node = node;
		this->cube = cube;
		this->context = context;
	}
};

struct StackFrame : public IteratorData {
	int childIndex; // Tracks which child is being processed
	int internalOrder[8]; // Stores child processing order
	bool secondVisit; // Tracks whether we are on the second visit

	StackFrame(IteratorData data, int childIndex, bool secondVisit) : IteratorData(data){
		this->childIndex = childIndex;
		this->secondVisit = secondVisit;
	}
};

struct StackFrameOut : public IteratorData  {
	bool visited;  // false: first time (push children), true: ready for after()

	StackFrameOut(IteratorData data, bool visited) : IteratorData(data){
		this->visited = visited;
	}
};




class IteratorHandler {
    std::stack<IteratorData> flatData;
    std::stack<StackFrame> stack;
    std::stack<StackFrameOut> stackOut;


	public: 
		virtual bool test(IteratorData &params) = 0;
		virtual void * before(IteratorData &params) = 0;
		virtual void after(IteratorData &params) = 0;
		virtual void getOrder(IteratorData &params, int * order) = 0;
		void iterate(IteratorData params);
		void iterateFlatIn(IteratorData params);
		void iterateFlatOut(IteratorData params);
		void iterateFlat(IteratorData params);
};


struct InstanceData {
    public:
    float shift;
    glm::mat4 matrix;

    InstanceData(glm::mat4 matrix,  float shift) {
        this->matrix = matrix;
        this->shift = shift;
    }
};

struct InstanceGeometry {
    public:
    Geometry * geometry;
    std::vector<InstanceData> instances;

    InstanceGeometry(Geometry * geometry);
};

class InstanceBuilderHandler {
	public:
	Octree * tree;
	long * count;
	
	InstanceBuilderHandler(Octree * tree, long * count);

	virtual void handle(OctreeNode *node, const BoundingCube &cube, int level, InstanceGeometry * pre) = 0;
};



class InstanceBuilder : public IteratorHandler{
	Octree * tree;
	Geometry chunk;
    uint mode;
	InstanceBuilderHandler * handler;
    public: 
        int instanceCount = 0;
        std::vector<InstanceData> * instances;
		InstanceBuilder(Octree * tree, std::vector<InstanceData> * instances, InstanceBuilderHandler * handler);

		void * before(IteratorData &params) override;
		void after(IteratorData &params) override;
		bool test(IteratorData &params) override;
		void getOrder(IteratorData &params, int * order) override;

};


class OctreeNodeTriangleInstanceBuilder : public OctreeNodeTriangleHandler {

	public: 
	std::vector<InstanceData> * instances;
    int pointsPerTriangle;

	using OctreeNodeTriangleHandler::OctreeNodeTriangleHandler;
	OctreeNodeTriangleInstanceBuilder(Geometry * chunk, long * count,std::vector<InstanceData> * instances, int pointsPerTriangle);
	void handle(OctreeNode* c0,OctreeNode* c1,OctreeNode* c2, bool sign) override;

};


class OctreeNodeTriangleTesselator : public OctreeNodeTriangleHandler {

	public: 
	using OctreeNodeTriangleHandler::OctreeNodeTriangleHandler;

	OctreeNodeTriangleTesselator(Geometry * chunk, long * count);
	void handle(OctreeNode* c0,OctreeNode* c1,OctreeNode* c2, bool sign) override;
};

class SphereGeometry : public Geometry{
    int lats;
    int longs;
public:
	SphereGeometry(int lats, int longs);
	void addTriangle(glm::vec3 a,glm::vec3 b, glm::vec3 c);

};

class Tesselator : public IteratorHandler{
	public:
		Octree * tree;
		long triangles;
		Geometry * chunk;
		int simplification;
		Tesselator(Octree * tree, Geometry * chunk, int simplification);
		void * before(IteratorData &params) override;
		void after(IteratorData &params) override;
		bool test(IteratorData &params) override;
		void getOrder(IteratorData &params, int * order) override;
};

class Simplifier : public IteratorHandler{
	public:
		Octree * tree;
		float angle;
		float distance;
	    bool texturing;
		int simplification;
		BoundingCube chunkCube;
		Simplifier(Octree * tree, BoundingCube chunkCube, float angle, float distance, bool texturing, int simplification);

		void * before(IteratorData &params) override;
		void after(IteratorData &params) override;
		bool test(IteratorData &params) override;
		void getOrder(IteratorData &params, int * order) override;

};


class Frustum
{
public:
	Frustum() {}

	// m = ProjectionMatrix * ViewMatrix 
	Frustum(glm::mat4 m);

	// http://iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
	bool isBoxVisible(const AbstractBoundingBox &box);

private:
	enum Planes
	{
		Left = 0,
		Right,
		Bottom,
		Top,
		Near,
		Far,
		Count,
		Combinations = Count * (Count - 1) / 2
	};

	template<Planes i, Planes j>
	struct ij2k
	{
		enum { k = i * (9 - i) / 2 + j - 1 };
	};

	template<Planes a, Planes b, Planes c> glm::vec3 intersection(glm::vec3* crosses);
	
	glm::vec4   m_planes[Count];
	glm::vec3   m_points[8];
};

class TexturePainter {
	public:
	virtual void paint(Vertex &v) const = 0;
};


class SphereContainmentHandler : public ContainmentHandler {
	public:
	BoundingSphere sphere;
    const TexturePainter &brush;

	SphereContainmentHandler(BoundingSphere s, const TexturePainter &b);
	glm::vec3 getCenter() const override;
	bool contains(const glm::vec3 p) const override;
	glm::vec3 getNormal(const glm::vec3 pos) const ;
	bool isContained(const BoundingCube &cube) const override;
	ContainmentType check(const BoundingCube &cube) const override;
	Vertex getVertex(const BoundingCube &cube, ContainmentType solid, glm::vec3 previousPoint) const override;
};

class BoxContainmentHandler : public ContainmentHandler {
	public: 
	BoundingBox box;
    const TexturePainter &brush;

	BoxContainmentHandler(BoundingBox box, const TexturePainter &b);
	glm::vec3 getCenter() const;
	bool contains(const glm::vec3 p) const ;
	bool isContained(const BoundingCube &cube) const override;
	ContainmentType check(const BoundingCube &cube) const override;
	Vertex getVertex(const BoundingCube &cube, ContainmentType solid, glm::vec3 previousPoint) const override;
};

class HeightMapContainmentHandler : public ContainmentHandler {
	public: 
	const HeightMap &map;
    const TexturePainter &brush;

	HeightMapContainmentHandler(const HeightMap &map, const TexturePainter &b);
	glm::vec3 getCenter() const;
	bool contains(const glm::vec3 p) const ;
	float intersection(const glm::vec3 a, const glm::vec3 b) const ;
	glm::vec3 getNormal(const glm::vec3 pos) const ;
	bool isContained(const BoundingCube &cube) const override;
	ContainmentType check(const BoundingCube &cube) const override;
	Vertex getVertex(const BoundingCube &cube, ContainmentType solid, glm::vec3 previousPoint) const override;
};

struct OctreeNodeSerialized {
    public:
    glm::vec3 position;
    glm::vec3 normal;
    uint brushIndex;
    uint mask;
    ContainmentType solid;
    uint children[8] = {0,0,0,0,0,0,0,0};
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
        void load();
};


class OctreeVisibilityChecker : public IteratorHandler{
	Geometry chunk;
	Frustum frustum;
    public: 
		Octree * tree;
	    std::vector<IteratorData> * visibleNodes;
        glm::vec3 sortPosition;

		OctreeVisibilityChecker(Octree * tree, std::vector<IteratorData> * visibleNodes);

		void update(glm::mat4 m);
		void * before(IteratorData &params) override;
		void after(IteratorData &params) override;
		bool test(IteratorData &params) override;
		void getOrder(IteratorData &params, int * order) override;

};


class OctreeProcessor : public IteratorHandler{
	Octree * tree;
	Geometry chunk;


    bool createInstances;
    public: 
		int loadCount = 0;
        glm::vec3 cameraPosition;
        GeometryBuilder * builder;
		OctreeProcessor(Octree * tree,bool createInstances, GeometryBuilder * builder);

		void * before(IteratorData &params) override;
		void after(IteratorData &params) override;
		bool test(IteratorData &params) override;
		void getOrder(IteratorData &params, int * order) override;

};

class Camera {
    public:
    glm::mat4 projection;
    glm::mat4 view;
    glm::quat quaternion;
    glm::vec3 position;
    float near;
    float far;

    Camera(float near, float far);
    glm::vec3 getCameraDirection();
    glm::mat4 getVP();
};

struct Tile {
    public:
    glm::vec2 size;
    glm::vec2 offset;
    
    Tile(glm::vec2 size, glm::vec2 offset);
};

struct TileDraw {
    public:
    glm::vec2 size;
    glm::vec2 offset;
    glm::vec2 pivot;
    float angle;
    uint index;
    
    TileDraw(uint index,glm::vec2 size, glm::vec2 offset, glm::vec2 pivot, float angle);
};


class Math
{
public:
	Math();
	~Math();
	static bool isBetween(float x, float min, float max);	
	static int clamp(int val, int min, int max);
	static float clamp(float val, float min, float max);
	static int triplanarPlane(glm::vec3 position, glm::vec3 normal);
	static int mod(int a, int b);
	static glm::vec2 triplanarMapping(glm::vec3 position, int plane);
	static glm::vec3 surfaceNormal(const glm::vec3 point, const BoundingBox &box);
	static glm::mat4 getCanonicalMVP(glm::mat4 m);
	static glm::mat4 getRotationMatrixFromNormal(glm::vec3 normal, glm::vec3 target);
};
void ensureFolderExists(const std::string& folder);

std::stringstream gzipDecompressFromIfstream(std::ifstream& inputFile);
void gzipCompressToOfstream(std::istream& inputStream, std::ofstream& outputFile);

#endif

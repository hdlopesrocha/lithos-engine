#ifndef MATH_HPP
#define MATH_HPP
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
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
#include "../lib/db_perlin.hpp"
#define INFO_TYPE_FILE 99
#define INFO_TYPE_REMOVE 0


class BoundingSphere;
class BoundingBox;

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
	float distance(glm::vec3 point);
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


class BoundingCube {
	private: 
		glm::vec3 min;
		float length;

	public: 
		BoundingCube();
		BoundingCube(glm::vec3 min, float length);
		float getMaxX();
		float getMaxY();
		float getMaxZ();
		float getMinX();
		float getMinY();
		float getMinZ();
		glm::vec3 getMin();
		glm::vec3 getMax();
		glm::vec3 getCenter();
		float getLength();
		void setLength(float l);
		void setMin(glm::vec3 v);
		bool contains(glm::vec3 point);
		bool contains(BoundingSphere sphere);
		bool contains(BoundingCube cube);
		bool contains(BoundingBox box);

};


class BoundingSphere {
	public: 
		glm::vec3 center;
		float radius;
		BoundingSphere();		
		BoundingSphere(glm::vec3 center, float radius);
		bool contains(glm::vec3 point);
		ContainmentType test(BoundingCube cube);
		bool intersects(BoundingCube cube);
};


class BoundingBox {
	private: 
		glm::vec3 min;
		glm::vec3 max;

	public: 
		BoundingBox(glm::vec3 min, glm::vec3 max);
		BoundingBox();
		float getMaxX();
		float getMaxY();
		float getMaxZ();
		float getMinX();
		float getMinY();
		float getMinZ();
		glm::vec3 getMin();
		glm::vec3 getMax();
		glm::vec3 getCenter();
		glm::vec3 getLength();
		void setMin(glm::vec3 v);
		void setMax(glm::vec3 v);
		ContainmentType test(BoundingCube cube);
		bool intersects(BoundingSphere sphere);
		bool contains(glm::vec3 point);
};


class HeightFunction {
	public:
		virtual float getHeightAt(float x, float y, float z) = 0;
		glm::vec3 getNormal(float x, float z, float delta);

};


class CachedHeightMapSurface : public HeightFunction {
	public:
		std::vector<std::vector<float>> data; 
		BoundingBox box;
		float delta;
		int width;
		int height;


	CachedHeightMapSurface(HeightFunction * function, BoundingBox box, float delta);
	float getData(int x, int z);
	float getHeightAt(float x, float yy, float z);

};

class PerlinSurface : public HeightFunction {
	public:
	float amplitude;
	float frequency;
	float offset;


	PerlinSurface(float amplitude, float frequency, float offset);
	float getHeightAt(float x, float y, float z);
};

class FractalPerlinSurface : public PerlinSurface {
	public:
	using PerlinSurface::PerlinSurface;
	FractalPerlinSurface(float amplitude, float frequency, float offset);
	float getHeightAt(float x, float y, float z);
};


class GradientPerlinSurface : public PerlinSurface {
	public:

	GradientPerlinSurface(float amplitude, float frequency, float offset);
	float getHeightAt(float x, float y, float z);
};

class HeightMap: public BoundingBox  {
	private: 
		float step;
		HeightFunction * func;
	public: 
		HeightMap(HeightFunction * func, glm::vec3 min, glm::vec3 max, float step);
		
   		glm::vec3 getNormalAt(float x, float z);
		glm::vec3 getPoint(BoundingCube cube);

		glm::ivec2 getIndexes(float x, float z);
		glm::vec2 getHeightRangeBetween(BoundingCube cube);
		bool hitsBoundary(BoundingCube cube);
		bool isContained(BoundingCube p);
		bool contains(glm::vec3 point);
		ContainmentType test(BoundingCube cube);

};

class ContainmentHandler {
	public: 
		virtual ContainmentType check(BoundingCube &cube) = 0;
		virtual Vertex getVertex(BoundingCube &cube, ContainmentType solid, glm::vec3 previousPoint) = 0;
		virtual glm::vec3 getCenter() = 0;
		virtual bool contains(glm::vec3 p) = 0;
		virtual bool isContained(BoundingCube &cube) = 0;
};

struct NodeInfo {
	public:
	void * data;
	void * temp;
	int type;
	bool dirty = false;

	NodeInfo(int type, void * data, void * temp, bool dirty) {
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
		NodeInfo * getNodeInfo(int infoType);
};

struct IteratorData {
	public:
	int level;
	OctreeNode * node;
	BoundingCube cube;
	void * context;

};

struct StackFrame {
	int level;
	OctreeNode* node;
	BoundingCube cube;
	void* context;
	int childIndex; // Tracks which child is being processed
	int internalOrder[8]; // Stores child processing order
	bool secondVisit; // Tracks whether we are on the second visit
};

struct StackFrameOut {
	int level;
	OctreeNode* node;
	BoundingCube cube;
	void* context;
	bool visited;  // false: first time (push children), true: ready for after()
};


class IteratorHandler {
    std::stack<IteratorData> flatData;
    std::stack<StackFrame> stack;
    std::stack<StackFrameOut> stackOut;


	public: 
		virtual bool test(int level, OctreeNode * node, BoundingCube &cube, void * context) = 0;
		virtual void * before(int level, OctreeNode * node, BoundingCube &cube, void * context) = 0;
		virtual void after(int level, OctreeNode * node, BoundingCube &cube, void * context) = 0;
		virtual void getOrder(BoundingCube &cube, int * order) = 0;
		void iterate(int level, OctreeNode * node, BoundingCube cube, void * context);
		void iterateFlatIn(int level, OctreeNode * node, BoundingCube cube, void * context);
		void iterateFlatOut(int level, OctreeNode * node, BoundingCube cube, void * context);
		void iterateFlat(int level, OctreeNode * root, BoundingCube cube, void * context);
};


class Geometry
{
public:
	std::vector<Vertex> vertices;
	std::vector<uint> indices;
	std::map <Vertex, int> compactMap;

	Geometry(/* args */);
	~Geometry();

	Vertex * addVertex(Vertex vertex);
	static glm::vec3 getNormal(Vertex * a, Vertex * b, Vertex * c);

};

class OctreeNodeTriangleHandler {

	public: 
	Geometry * chunk;
	int * count;
	OctreeNodeTriangleHandler(Geometry * chunk, int * count);
	virtual void handle(OctreeNode* c0,OctreeNode* c1,OctreeNode* c2, bool sign) = 0;
};

class OctreeNodeTriangleTesselator : public OctreeNodeTriangleHandler {

	public: 
	using OctreeNodeTriangleHandler::OctreeNodeTriangleHandler;

	OctreeNodeTriangleTesselator(Geometry * chunk, int * count);
	void handle(OctreeNode* c0,OctreeNode* c1,OctreeNode* c2, bool sign);
};

class Octree: public BoundingCube {
	public: 
		float minSize;
		OctreeNode * root;

		Octree(BoundingCube minCube);
		void expand(ContainmentHandler * handler);
		void add(ContainmentHandler * handler);
		void del(ContainmentHandler * handler);
		void iterate(IteratorHandler * handler);
		void iterateFlat(IteratorHandler * handler);

		OctreeNode* getNodeAt(const glm::vec3 &pos, int level, int simplification);
		void handleQuadNodes(OctreeNode * node, OctreeNode** corners, OctreeNodeTriangleHandler * handler);
		void getNodeNeighbors(BoundingCube &cube, int level, int simplification, int direction, OctreeNode ** out, int initialIndex, int finalIndex);

		static glm::vec3 getShift(int i);
		static glm::vec3 getShift3(int i);
		static BoundingCube getChildCube(BoundingCube &cube, int i);
		static BoundingCube getCube3(BoundingCube &cube, int i);

		int getHeight(BoundingCube  &cube);

};

class SphereGeometry : public Geometry{
    int lats;
    int longs;
public:
	SphereGeometry(int lats, int longs);

};

class Tesselator : public IteratorHandler{
	public:
		Octree * tree;
		int triangles;
		Geometry * chunk;
		int simplification;
		Tesselator(Octree * tree, Geometry * chunk, int simplification);
		void * before(int level, OctreeNode * node, BoundingCube &cube, void * context) override;
		void after(int level, OctreeNode * node, BoundingCube &cube, void * context) override;
		bool test(int level, OctreeNode * node, BoundingCube &cube, void * context) override;
		void getOrder(BoundingCube &cube, int * order) override;
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

		void * before(int level, OctreeNode * node, BoundingCube &cube, void * context) override;
		void after(int level, OctreeNode * node, BoundingCube &cube, void * context) override;
		bool test(int level, OctreeNode * node, BoundingCube &cube, void * context) override;
		void getOrder(BoundingCube &cube, int * order) override;

};


class Frustum
{
public:
	Frustum() {}

	// m = ProjectionMatrix * ViewMatrix 
	Frustum(glm::mat4 m);

	// http://iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
	bool isBoxVisible(BoundingBox box);

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

class TextureBrush {
	public:
	virtual void paint(Vertex * v) = 0;
};


class SphereContainmentHandler : public ContainmentHandler {
	public:
	BoundingSphere sphere;
    TextureBrush * brush;

	SphereContainmentHandler(BoundingSphere s, TextureBrush * b);
	glm::vec3 getCenter();
	bool contains(glm::vec3 p);
	glm::vec3 getNormal(glm::vec3 pos);
	bool isContained(BoundingCube &cube) override;
	ContainmentType check(BoundingCube &cube) override;
	Vertex getVertex(BoundingCube &cube, ContainmentType solid, glm::vec3 previousPoint) override;
};

class BoxContainmentHandler : public ContainmentHandler {
	public: 
	BoundingBox box;
    TextureBrush * brush;

	BoxContainmentHandler(BoundingBox box, TextureBrush * b);
	glm::vec3 getCenter();
	bool contains(glm::vec3 p);
	bool isContained(BoundingCube &cube) override;
	ContainmentType check(BoundingCube &cube) override;
	Vertex getVertex(BoundingCube &cube, ContainmentType solid, glm::vec3 previousPoint) override;
};

class HeightMapContainmentHandler : public ContainmentHandler {
	public: 
	HeightMap * map;
    TextureBrush * brush;

	HeightMapContainmentHandler(HeightMap * m, TextureBrush * b);
	glm::vec3 getCenter();
	bool contains(glm::vec3 p);
	float intersection(glm::vec3 a, glm::vec3 b);
	glm::vec3 getNormal(glm::vec3 pos);
	bool isContained(BoundingCube &cube) override;
	ContainmentType check(BoundingCube &cube) override;
	Vertex getVertex(BoundingCube &cube, ContainmentType solid, glm::vec3 previousPoint) override;
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
		int geometryLevel;
        glm::vec3 sortPosition;

		OctreeVisibilityChecker(Octree * tree, int geometryLevel, std::vector<IteratorData> * visibleNodes);

		void update(glm::mat4 m);
		void * before(int level, OctreeNode * node, BoundingCube &cube, void * context) override;
		void after(int level, OctreeNode * node, BoundingCube &cube, void * context) override;
		bool test(int level, OctreeNode * node, BoundingCube &cube, void * context) override;
		void getOrder(BoundingCube &cube, int * order) override;

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
	static glm::vec3 surfaceNormal(glm::vec3 point, BoundingBox &box);
	static glm::mat4 getCanonicalMVP(glm::mat4 m);

};
void ensureFolderExists(const std::string& folder);

std::stringstream gzipDecompressFromIfstream(std::ifstream& inputFile);
void gzipCompressToOfstream(std::istream& inputStream, std::ofstream& outputFile);

#endif

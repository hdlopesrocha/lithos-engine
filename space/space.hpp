#ifndef SPACE_HPP
#define SPACE_HPP

#include "../math/math.hpp"
#include "Allocator.hpp"

class Octree;
class OctreeAllocator;

struct ChildBlock {
	uint children[8]; 

	public:
	ChildBlock();
	ChildBlock * init();
	void clear(OctreeAllocator * allocator);
};

class OctreeNode {
	ChildBlock * block;

	public: 
		Vertex vertex;
		uint8_t mask;
		uint8_t bits;
		long dataId;

		OctreeNode(Vertex vertex);
		~OctreeNode();
		OctreeNode * init(Vertex vertex);
		void clear(OctreeAllocator * allocator);
		void setChildNode(int i, OctreeNode * node, OctreeAllocator * allocator);
		OctreeNode * getChildNode(int i, OctreeAllocator * allocator);
		bool isLeaf();
		bool isSolid();
		void setSolid(bool value);
		bool isSimplified();
		void setSimplified(bool value);
		bool isDirty();
		void setDirty(bool value);

};

class OctreeNodeTriangleHandler {

	public: 
	long * count;
	OctreeNodeTriangleHandler(long * count);
	virtual void handle(OctreeNode* c0,OctreeNode* c1,OctreeNode* c2, bool sign) = 0;
};


struct OctreeNodeData {
	public:
	uint level;
	float chunkSize;
	OctreeNode * node;
	BoundingCube cube;
	void * context;
	OctreeAllocator * allocator;
	OctreeNodeData(uint level, float chunkSize, OctreeNode * node, BoundingCube cube, void * context, OctreeAllocator * allocator) {
		this->level = level;
		this->chunkSize = chunkSize;
		this->node = node;
		this->cube = cube;
		this->context = context;
		this->allocator = allocator;
	}
};

class OctreeNodeDirtyHandler {
	public:

	virtual void handle(OctreeNode * node) const = 0;
};

class OctreeAllocator {
	public: 
	Allocator<OctreeNode> nodeAllocator;
	Allocator<ChildBlock> childAllocator;

};

class Octree: public BoundingCube {
	using BoundingCube::BoundingCube;
	public: 
		long dataId;
		OctreeNode * root;
		OctreeAllocator allocator;

		Octree(BoundingCube minCube);
		void expand(const ContainmentHandler &handler);
		void add(const ContainmentHandler &handler, const OctreeNodeDirtyHandler &dirtyHandler, float minSize);
		void del(const ContainmentHandler &handler, const OctreeNodeDirtyHandler &dirtyHandler, float minSize);
		void iterate(IteratorHandler &handler, float chunkSize);
		void iterateFlat(IteratorHandler &handler, float chunkSize);

		OctreeNode* getNodeAt(const glm::vec3 &pos, int level, bool simplification);
		OctreeNode* getNodeAt(const glm::vec3 &pos, bool simplification);
		void handleQuadNodes(OctreeNodeData &data, OctreeNodeTriangleHandler * handler, bool simplification);
		void getNodeNeighbors(OctreeNodeData &data, bool simplification, OctreeNode ** out, int direction,int initialIndex, int finalIndex);
		ContainmentType contains(const glm::vec3 &pos);
		ContainmentType contains(const AbstractBoundingBox&cube);
		bool hasFinerNode(const OctreeNode *node);
		int getLevelAt(const glm::vec3 &pos, bool simplification);
		int getNeighborLevel(OctreeNodeData &data, bool simplification, int direction);
		OctreeNode * fetch(OctreeNodeData &data, OctreeNode ** out, int i);
		int getMaxLevel(OctreeNode * node, int level);
};

struct StackFrame : public OctreeNodeData {
	uint8_t childIndex; // Tracks which child is being processed
	uint8_t internalOrder[8]; // Stores child processing order
	bool secondVisit; // Tracks whether we are on the second visit

	StackFrame(const OctreeNodeData &data, uint8_t childIndex, bool secondVisit) : OctreeNodeData(data){
		this->childIndex = childIndex;
		this->secondVisit = secondVisit;
	}
};

struct StackFrameOut : public OctreeNodeData  {
	bool visited;  // false: first time (push children), true: ready for after()

	StackFrameOut(const OctreeNodeData &data, bool visited) : OctreeNodeData(data){
		this->visited = visited;
	}
};

class GeometryBuilder {
    public:
    virtual InstanceGeometry* build(OctreeNodeData &params) = 0;
};

class MeshGeometryBuilder  : public GeometryBuilder {
    float simplificationAngle;
    float simplificationDistance;
    bool simplificationTexturing;
	long * trianglesCount;
    Octree * tree;
	public:
		MeshGeometryBuilder(long * trianglesCount,Octree * tree, float simplificationAngle, float simplificationDistance, bool simplificationTexturing);
		~MeshGeometryBuilder();
		InstanceGeometry * build(OctreeNodeData &params) override;
};

class IteratorHandler {
    std::stack<OctreeNodeData> flatData;
    std::stack<StackFrame> stack;
    std::stack<StackFrameOut> stackOut;
	public: 
		virtual bool test(OctreeNodeData &params) = 0;
		virtual void before(OctreeNodeData &params) = 0;
		virtual void after(OctreeNodeData &params) = 0;
		virtual void getOrder(OctreeNodeData &params, uint8_t * order) = 0;
		void iterate(OctreeNodeData params);
		void iterateFlatIn(OctreeNodeData params);
		void iterateFlatOut(OctreeNodeData params);
		void iterateFlat(OctreeNodeData params);
};

class InstanceBuilderHandler {
	public:
		InstanceBuilderHandler();
		virtual void handle(OctreeNodeData &data, std::vector<InstanceData> * instances) = 0;
};

class InstanceBuilder : public IteratorHandler{
	InstanceBuilderHandler * handler;
	std::vector<InstanceData> * instances;
	public: 
		InstanceBuilder(InstanceBuilderHandler * handler, std::vector<InstanceData> * instances);
		void before(OctreeNodeData &params) override;
		void after(OctreeNodeData &params) override;
		bool test(OctreeNodeData &params) override;
		void getOrder(OctreeNodeData &params, uint8_t * order) override;

};

class Tesselator : public IteratorHandler, OctreeNodeTriangleHandler{
	Octree * tree;
	Geometry * chunk;
	public:
		Tesselator(Octree * tree, Geometry * chunk, long * count);
		void before(OctreeNodeData &params) override;
		void after(OctreeNodeData &params) override;
		bool test(OctreeNodeData &params) override;
		void getOrder(OctreeNodeData &params, uint8_t * order) override;
		void handle(OctreeNode* c0,OctreeNode* c1,OctreeNode* c2, bool sign) override;
		void virtualize(OctreeNodeData &params, int levels);

};

class Simplifier : public IteratorHandler{
	Octree * tree;
	float angle;
	float distance;
	bool texturing;
	BoundingCube chunkCube;
	public:
		Simplifier(Octree * tree, BoundingCube chunkCube, float angle, float distance, bool texturing);
		void before(OctreeNodeData &params) override;
		void after(OctreeNodeData &params) override;
		bool test(OctreeNodeData &params) override;
		void getOrder(OctreeNodeData &params, uint8_t * order) override;

};

struct OctreeNodeSerialized {
    public:
    glm::vec3 position;
    glm::vec3 normal;
    uint brushIndex;
    uint mask;
    bool isSolid;
    uint children[8] = {0,0,0,0,0,0,0,0};
};

struct OctreeSerialized {
    public:
    glm::vec3 min;
    float length;
};

class OctreeFile {
	Octree * tree;
    std::string filename;
    int chunkHeight;
    public: 
		OctreeFile(Octree * tree, std::string filename, int chunkHeight);
        void save(std::string baseFolder, float chunkSize);
        void load(std::string baseFolder, float chunkSize);
		AbstractBoundingBox& getBox();

};

class OctreeNodeFile {
	OctreeNode * node;
    std::string filename;
    public: 
		OctreeNodeFile(OctreeNode * node, std::string filename);
        void save(OctreeAllocator * allocator, std::string baseFolder);
        void load(OctreeAllocator * allocator, std::string baseFolder);
};


class OctreeVisibilityChecker : public IteratorHandler{
	Frustum frustum;
	public:
		glm::vec3 sortPosition;
		std::vector<OctreeNodeData> * visibleNodes;
		OctreeVisibilityChecker(std::vector<OctreeNodeData> * visibleNodes);
		void update(glm::mat4 m);
		void before(OctreeNodeData &params) override;
		void after(OctreeNodeData &params) override;
		bool test(OctreeNodeData &params) override;
		void getOrder(OctreeNodeData &params, uint8_t * order) override;

};


class OctreeProcessor {
	Octree * tree;
    public: 
		OctreeProcessor(Octree * tree);
		bool process(OctreeNodeData &params);
};

#endif
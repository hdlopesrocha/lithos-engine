#ifndef SPACE_HPP
#define SPACE_HPP

#include "../math/math.hpp"
class OctreeNodeAllocator;

class OctreeNode {

	public: 
		Vertex vertex;
		bool simplified;
		uint mask;
		bool isSolid;
		long dataId;
		OctreeNode *children[8];

		OctreeNode(Vertex vertex);
		~OctreeNode();
		OctreeNode * init(Vertex vertex);
		void clear(OctreeNodeAllocator * allocator);
		void setChildNode(int i, OctreeNode * node);
		OctreeNode * getChildNode(int i);
		bool isLeaf();
};

class OctreeNodeAllocator {
	std::vector<OctreeNode*> freeList; 
	std::vector<OctreeNode*> allocatedBlocks;
	size_t blockSize; 
	void allocateBlock();
	
	public:
        OctreeNodeAllocator();
        ~OctreeNodeAllocator();

        OctreeNode* allocate();
        void deallocate(OctreeNode* ptr);
};

class OctreeNodeTriangleHandler {

	public: 
	Geometry * chunk;
	long * count;
	OctreeNodeTriangleHandler(Geometry * chunk, long * count);
	virtual void handle(OctreeNode* c0,OctreeNode* c1,OctreeNode* c2, bool sign) = 0;
};


struct OctreeNodeData {
	public:
	int level;
	float chunkSize;
	OctreeNode * node;
	BoundingCube cube;
	void * context;
	OctreeNodeData(int level, float chunkSize, OctreeNode * node, BoundingCube cube, void * context) {
		this->level = level;
		this->chunkSize = chunkSize;
		this->node = node;
		this->cube = cube;
		this->context = context;
	}
};

class OctreeNodeDirtyHandler {
	public:

	virtual void handle(long dataId) const = 0;
};

class Octree: public BoundingCube {
	using BoundingCube::BoundingCube;
	public: 
		long dataId;
		OctreeNode * root;
		OctreeNodeAllocator allocator;

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
	int childIndex; // Tracks which child is being processed
	int internalOrder[8]; // Stores child processing order
	bool secondVisit; // Tracks whether we are on the second visit

	StackFrame(const OctreeNodeData &data, int childIndex, bool secondVisit) : OctreeNodeData(data){
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
	long * instancesCount;
	long * trianglesCount;
    Octree * tree;
	public:
		MeshGeometryBuilder(long * instancesCount, long * trianglesCount,Octree * tree, float simplificationAngle, float simplificationDistance, bool simplificationTexturing);
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
		virtual void getOrder(OctreeNodeData &params, int * order) = 0;
		void iterate(OctreeNodeData params);
		void iterateFlatIn(OctreeNodeData params);
		void iterateFlatOut(OctreeNodeData params);
		void iterateFlat(OctreeNodeData params);
};

class InstanceBuilderHandler {
	public:
		Octree * tree;
		long * count;
		InstanceBuilderHandler(Octree * tree, long * count);
		virtual void handle(OctreeNodeData &data, InstanceGeometry * pre) = 0;
};

class InstanceBuilder : public IteratorHandler{
	Octree * tree;
    uint mode;
	InstanceBuilderHandler * handler;
	InstanceGeometry * geometry;
	std::vector<InstanceData> * instances;
	public: 
		int instanceCount = 0;
		InstanceBuilder(Octree * tree, std::vector<InstanceData> * instances, InstanceBuilderHandler * handler, InstanceGeometry * geometry);
		void before(OctreeNodeData &params) override;
		void after(OctreeNodeData &params) override;
		bool test(OctreeNodeData &params) override;
		void getOrder(OctreeNodeData &params, int * order) override;

};

class Tesselator : public IteratorHandler, OctreeNodeTriangleHandler{
	Octree * tree;
	public:
		Tesselator(Octree * tree, Geometry * chunk, long * count);
		void before(OctreeNodeData &params) override;
		void after(OctreeNodeData &params) override;
		bool test(OctreeNodeData &params) override;
		void getOrder(OctreeNodeData &params, int * order) override;
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
		void getOrder(OctreeNodeData &params, int * order) override;

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
        void save(std::string baseFolder);
        void load(Octree * tree, std::string baseFolder);
};


class OctreeVisibilityChecker : public IteratorHandler{
	Frustum frustum;
	public:
		glm::vec3 sortPosition;
		std::vector<OctreeNodeData> * visibleNodes;
		Octree * tree;
		OctreeVisibilityChecker(Octree * tree, std::vector<OctreeNodeData> * visibleNodes);
		void update(glm::mat4 m);
		void before(OctreeNodeData &params) override;
		void after(OctreeNodeData &params) override;
		bool test(OctreeNodeData &params) override;
		void getOrder(OctreeNodeData &params, int * order) override;

};


class OctreeProcessor {
	Octree * tree;
    public: 
		int loadCount = 0;
		OctreeProcessor(Octree * tree);
		void process(OctreeNodeData &params);
};

#endif
#ifndef SPACE_HPP
#define SPACE_HPP

#include "../math/math.hpp"
#include "../math/SDF.hpp"
#include "Allocator.hpp"
#define SQRT_3_OVER_2 0.866025404f

class Octree;
class OctreeAllocator;
class Simplifier;
struct ChildBlock;

class OctreeNode {

	public: 
		Vertex vertex;
		uint16_t bits;
		uint id;
		float sdf[8];

		OctreeNode(Vertex vertex);
		~OctreeNode();
		OctreeNode * init(Vertex vertex);
		void clear(OctreeAllocator * allocator, BoundingCube &cube);
		void setChildNode(int i, OctreeNode * node, OctreeAllocator * allocator, ChildBlock * block);
		ChildBlock * getBlock(OctreeAllocator * allocator);
		ChildBlock * createBlock(OctreeAllocator * allocator);
		OctreeNode * getChildNode(int i, OctreeAllocator * allocator, ChildBlock * block);
		bool isLeaf();
		void setSimplification(uint8_t value);
		uint8_t getSimplification();
		bool isSolid();
		void setSolid(bool value);
		bool isSimplified();
		void setSimplified(bool value);
		bool isDirty();
		void setDirty(bool value);
		void setSdf(float * value);

};

struct ChildBlock {
	uint children[8]; 

	public:
	ChildBlock();
	ChildBlock * init();
	void clear(OctreeAllocator * allocator, BoundingCube &cube);

	void set(int i, OctreeNode * node, OctreeAllocator * allocator);
	OctreeNode * get(int i, OctreeAllocator * allocator);
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
	Allocator<OctreeNode> nodeAllocator;
	std::unordered_map<BoundingCube, OctreeNode*, BoundingCubeKeyHash> compactMap;

	public: 
	Allocator<ChildBlock> childAllocator;
	OctreeNode * allocateOctreeNode(BoundingCube &cube);
	OctreeNode * getOctreeNode(uint index);
	OctreeNode * getOctreeNode(BoundingCube &cube);
	void deallocateOctreeNode(OctreeNode * node, BoundingCube &cube);
	uint getIndex(OctreeNode * node);
    size_t getBlockSize() const;
    size_t getAllocatedBlocksCount() const ;

};

struct OctreeNodeFrame {
    OctreeNode* node;
    int childIndex;
    BoundingCube cube;
    float minSize;
	uint level;
	float sdf[8];
	SpaceType type;

	OctreeNodeFrame(OctreeNode* node, int childIndex, BoundingCube cube, float minSize, uint level, float * sdf, SpaceType type) 
		: node(node), childIndex(childIndex), cube(cube), minSize(minSize), level(level), type(type) {
			for(int i = 0; i < 8; ++i) {
				this->sdf[i] = sdf!=NULL ? sdf[i] : 0.0f;
			}	
	}
};

struct NodeOperationResult {
    OctreeNode * node;
	BoundingCube cube;
	SpaceType type;
	bool process;
    float sdf[8];


	NodeOperationResult() : node(NULL), cube(glm::vec3(0.0f), 0.0f), type(SpaceType::Empty), process(false) {
	};

    NodeOperationResult(BoundingCube cube, OctreeNode * node, SpaceType type, float * sdf, bool process) 
        : node(node), cube(cube), type(type), process(process){
		if(sdf != NULL) {
			for(int i = 0; i < 8; ++i) {
				this->sdf[i] = sdf[i];
			}	
		}					
    };
};


class Octree: public BoundingCube {
	using BoundingCube::BoundingCube;
	public: 
		float chunkSize;
		OctreeNode * root;
		OctreeAllocator allocator;

		Octree(BoundingCube minCube, float chunkSize);
		void expand(const ContainmentHandler &handler);
		void add(const ContainmentHandler &handler, const SignedDistanceFunction &function, const TexturePainter &painter, const OctreeNodeDirtyHandler &dirtyHandler, float minSize, Simplifier &simplifier);
		void del(const ContainmentHandler &handler, const SignedDistanceFunction &function, const TexturePainter &painter, const OctreeNodeDirtyHandler &dirtyHandler, float minSize, Simplifier &simplifier);
		void iterate(IteratorHandler &handler);
		void iterateFlat(IteratorHandler &handler);

		OctreeNode* getNodeAt(const glm::vec3 &pos, int level, bool simplification);
		OctreeNode* getNodeAt(const glm::vec3 &pos, bool simplification);
		void handleQuadNodes(OctreeNodeData &data, OctreeNodeTriangleHandler * handler, bool simplification);
		ContainmentType contains(const glm::vec3 &pos);
		ContainmentType contains(const AbstractBoundingBox&cube);
		bool hasFinerNode(const OctreeNode *node);
		int getLevelAt(const glm::vec3 &pos, bool simplification);
		int getNeighborLevel(OctreeNodeData &data, bool simplification, int direction);
		OctreeNode * fetch(OctreeNodeData &data, OctreeNode ** out, int i);
		int getMaxLevel(OctreeNode * node, int level);

		int getMaxLevel(BoundingCube &cube);
		int getMaxLevel(OctreeNode *node, BoundingCube &cube, BoundingCube &c, int level);

		private:
		NodeOperationResult shape(float (*operation)(float, float), const ContainmentHandler &handler, const SignedDistanceFunction &function, const TexturePainter &painter, const OctreeNodeDirtyHandler &dirtyHandler, OctreeNodeFrame frame, BoundingCube * chunk, Simplifier &simplifier);
};

class Simplifier {
	float angle;
	float distance;
	bool texturing;
	public:
		Simplifier(float angle, float distance, bool texturing);
		void simplify(BoundingCube chunkCube, const OctreeNodeData &params);

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

struct InstanceData {
    public:
    float shift;
	uint animation;
    glm::mat4 matrix;

    InstanceData(uint animation, glm::mat4 matrix, float shift) {
        this->matrix = matrix;
        this->shift = shift;
		this->animation = animation;
    }
};

struct DebugInstanceData {
    public:
	float sdf[8];
    glm::mat4 matrix;

    DebugInstanceData(glm::mat4 matrix, float * sdf) {
		for(int i = 0; i < 8; ++i) {
			this->sdf[i] = sdf[i];
		}
        this->matrix = matrix;
    }
};

template <typename T> class GeometryBuilder {
    public:
    virtual InstanceGeometry<T> * build(OctreeNodeData &params) = 0;
};

class MeshGeometryBuilder  : public GeometryBuilder<InstanceData> {
	long * trianglesCount;
    Octree * tree;
	public:
		MeshGeometryBuilder(long * trianglesCount, Octree * tree);
		~MeshGeometryBuilder();
		InstanceGeometry<InstanceData> * build(OctreeNodeData &params) override;
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

template <typename T> class InstanceBuilderHandler {
	public:
		virtual void handle(OctreeNodeData &data, std::vector<T> * instances) = 0;
};

template <typename T> class InstanceBuilder : public IteratorHandler{
	InstanceBuilderHandler<T> * handler;
	std::vector<T> * instances;
	public: 

		InstanceBuilder(InstanceBuilderHandler<T> * handler, std::vector<T> * instances) {
			this->instances = instances;
			this->handler = handler;
		}
		
		void before(OctreeNodeData &params) {		
			handler->handle(params, instances);
		}
		
		void after(OctreeNodeData &params) {			
			return;
		}
		
		bool test(OctreeNodeData &params) {	
			return true;
		}
		
		void getOrder(OctreeNodeData &params, uint8_t * order){
			for(size_t i = 0 ; i < 8 ; ++i) {
				order[i] = i;
			}
		}
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


struct OctreeNodeSerialized {
    public:
    glm::vec3 position;
    glm::vec3 normal;
    uint brushIndex;
    uint mask;
    bool isSolid;
	float sdf[8];
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
        void load(OctreeAllocator * allocator, std::string baseFolder, BoundingCube &cube);
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

#endif
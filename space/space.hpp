#ifndef SPACE_HPP
#define SPACE_HPP
#include <semaphore>
#include <thread>
#include <unordered_set>
#include <utility>
#include "../math/math.hpp"
#include "../math/SDF.hpp"
#include "Allocator.hpp"
#define SQRT_3_OVER_2 0.866025404f

class Octree;
class OctreeNode;
class OctreeAllocator;
class Simplifier;
struct ChildBlock;

#pragma pack(16)  // Ensure 16-byte alignment for UBO
struct OctreeSerialized {
    public:
    glm::vec3 min;
    float length;
	float chunkSize;
};
#pragma pack()  // Reset to default packing



struct alignas(16) OctreeNodeCubeSerialized {
    glm::vec4 position;
    glm::vec4 normal;
    glm::vec2 texCoord;
    int brushIndex;
	uint sign;
    uint children[8];
    glm::vec3 min;
    uint bits;
    glm::vec3 length;
	uint level;

	OctreeNodeCubeSerialized();
	OctreeNodeCubeSerialized(float * sdf, BoundingCube cube, Vertex vertex, uint bits, uint level) {
		this->sign = 0u;
		for(int i = 0; i < 8; ++i) {
			this->children[i] = 0;
			this->sign |= sdf[i] < 0.0f ? (1u << i) : 0;
		}
		this->position = vertex.position;
		this->normal = vertex.normal;
		this->texCoord = vertex.texCoord;
		this->brushIndex = vertex.brushIndex;
		this->min = cube.getMin();
		this->length = cube.getLength();
		this->bits = bits;
		this->level = level;
	};
};


#pragma pack(16)  // Ensure 16-byte alignment for UBO
struct OctreeNodeSerialized {
    public:
	float sdf[8];
    uint children[8] = {0,0,0,0,0,0,0,0};
    int brushIndex;
    uint bits;

	bool isDirty(){
		return this->bits & (0x1 << 3);
	}

	void setDirty(bool value){
		uint8_t mask = (0x1 << 3);
		this->bits = (this->bits & ~mask) | (value ? mask : 0x0);
	}

};
#pragma pack()  // Reset to default packing

class OctreeChangeHandler {
	public:
	virtual void create(OctreeNode* nodeId) = 0;
	virtual void update(OctreeNode* nodeId) = 0;
	virtual void erase(OctreeNode* nodeId) = 0;
};

class OctreeNode {

	public: 
		Vertex vertex;
		uint8_t bits;
		uint id;
		uint mask;
		float sdf[8];

		OctreeNode(Vertex vertex);
		~OctreeNode();
		OctreeNode * init(Vertex vertex);
		void clear(OctreeAllocator * allocator, BoundingCube &cube, OctreeChangeHandler * handler);
		void setChildNode(int i, OctreeNode * node, OctreeAllocator * allocator, ChildBlock * block);
		ChildBlock * getBlock(OctreeAllocator * allocator);
		ChildBlock * createBlock(OctreeAllocator * allocator);
		OctreeNode * getChildNode(int i, OctreeAllocator * allocator, ChildBlock * block);
		
		bool isSolid();
		void setSolid(bool value);
		
		bool isEmpty();
		void setEmpty(bool value);
		
		bool isSimplified();
		void setSimplified(bool value);
		
		bool isDirty();
		void setDirty(bool value);
		
		bool isChunk();
		void setChunk(bool value);
		
		bool isLeaf();
		void setLeaf(bool value);

		void setSdf(float * value);
		uint exportSerialization(OctreeAllocator * allocator, std::vector<OctreeNodeCubeSerialized> * nodes, int * leafNodes, BoundingCube cube, BoundingCube chunk, uint level);
		OctreeNode * compress(OctreeAllocator * allocator, BoundingCube * cube, BoundingCube chunk);
};

struct ChildBlock {
	uint children[8]; 

	public:
	ChildBlock();
	ChildBlock * init();
	void clear(OctreeAllocator * allocator, BoundingCube &cube, OctreeChangeHandler * handler);

	void set(int i, OctreeNode * node, OctreeAllocator * allocator);
	OctreeNode * get(int i, OctreeAllocator * allocator);
};

class OctreeNodeTriangleHandler {

	public: 
	long * count;
	OctreeNodeTriangleHandler(long * count);
	virtual void handle(Vertex &v0, Vertex &v1, Vertex &v2, bool sign) = 0;
};


struct OctreeNodeData {
	public:
	uint level;
	OctreeNode * node;
	BoundingCube cube;
	void * context;
	OctreeNodeData(uint level, OctreeNode * node, BoundingCube cube, void * context) {
		this->level = level;
		this->node = node;
		this->cube = cube;
		this->context = context;
	}
};


class OctreeAllocator {
	Allocator<OctreeNode> nodeAllocator;
	std::unordered_map<BoundingCube, OctreeNode*, BoundingCubeKeyHash> compactMap;
	std::mutex mtx;
	
	public: 
	Allocator<ChildBlock> childAllocator;
	OctreeNode * allocateOctreeNode(BoundingCube &cube);
	OctreeNode * getOctreeNode(uint index);
	OctreeNode * getOctreeNode(BoundingCube &cube);
	void deallocateOctreeNode(OctreeNode * node, BoundingCube &cube);
	uint getIndex(OctreeNode * node);
    size_t getBlockSize() const;
    size_t getAllocatedBlocksCount() ;

};

struct OctreeNodeFrame {
    OctreeNode* node;
    BoundingCube cube;
    float minSize;
	uint level;
	float sdf[8];
	SpaceType type;
	OctreeNodeFrame(OctreeNode* node, BoundingCube cube, float minSize, uint level, float * sdf, SpaceType type) 
		: node(node), cube(cube), minSize(minSize), level(level), type(type) {
			for(int i = 0; i < 8; ++i) {
				this->sdf[i] = sdf!=NULL ? sdf[i] : 0.0f;
			}	
	}
};

struct NodeOperationResult {
    OctreeNode * node;
	BoundingCube cube;
	SpaceType resultType;
	SpaceType shapeType;
	bool process;
    float sdf[8];
    float shapeSDF[8];
	bool isLeaf;

	NodeOperationResult() : node(NULL), cube(glm::vec3(0.0f), 0.0f), resultType(SpaceType::Empty), shapeType(SpaceType::Empty), process(false), isLeaf(false) {
		for(int i = 0; i < 8; ++i) {
			this->sdf[i] = INFINITY;
			this->shapeSDF[i] = INFINITY;
		}
	};

    NodeOperationResult(BoundingCube cube, OctreeNode * node, SpaceType shapeType, SpaceType resultType, float * sdf, float * shapeSDF, bool isLeaf, bool process) 
        : node(node), cube(cube), resultType(resultType), shapeType(shapeType), process(process), isLeaf(isLeaf) {
		if(sdf != NULL) {
			SDF::copySDF(sdf, this->sdf);	
		}
		if(shapeSDF != NULL) {
			SDF::copySDF(shapeSDF, this->shapeSDF);	
		}					
    };
};

struct ShapeContext {
	int threadId;

	ShapeContext(	int threadId) : threadId(threadId)  {

	}
};

struct ShapeArgs {
    float (*operation)(float, float);
    WrappedSignedDistanceFunction * function; 
    const TexturePainter &painter;
    const Transformation model;
    OctreeNodeFrame frame;
    Simplifier &simplifier; 
    OctreeChangeHandler * changeHandler;
    ShapeArgs(
        float (*operation)(float, float),
        WrappedSignedDistanceFunction * function, 
        const TexturePainter &painter,
        const Transformation model,
        OctreeNodeFrame frame, 
        Simplifier &simplifier, 
        OctreeChangeHandler * changeHandler
	) :
        operation(operation),
        function(function),
        painter(painter),
        model(model),
        frame(frame),
        simplifier(simplifier),
        changeHandler(changeHandler)

		 {

            
        };


};

struct ShapeChildArgs {
    NodeOperationResult * children;
    std::atomic<int> &childResultSolid;
    std::atomic<int> &childResultEmpty;
    std::atomic<int> &childShapeSolid;
    std::atomic<int> &childShapeEmpty;
    std::atomic<int> &childProcess;
    ChildBlock * block;
    int i;

    ShapeChildArgs(
        NodeOperationResult * children,
        std::atomic<int> &childResultSolid,
        std::atomic<int> &childResultEmpty,
        std::atomic<int> &childShapeSolid,
        std::atomic<int> &childShapeEmpty,
        std::atomic<int> &childProcess,
        ChildBlock * block, 
        int i
	) :
	    children(children),
        childResultSolid(childResultSolid),
        childResultEmpty(childResultEmpty),
        childShapeSolid(childShapeSolid),
        childShapeEmpty(childShapeEmpty),
        childProcess(childProcess),
        block(block),
        i(i)

		 {

            
        };


};

class ChunkContext {
	public:
	std::unordered_map<glm::vec3, float> sdfCache;
    BoundingCube cube;

	ChunkContext() : cube(BoundingCube()) {
	}
	
	ChunkContext(BoundingCube cube) : cube(cube) {
	}
};



class Octree: public BoundingCube {
	public: 
		float chunkSize;
		OctreeNode * root;
		OctreeAllocator allocator;
		std::shared_ptr<std::atomic<int>> counter = std::make_shared<std::atomic<int>>(0);
		std::shared_ptr<std::atomic<int>> threadId = std::make_shared<std::atomic<int>>(0);
		std::shared_ptr<std::atomic<int>> works = std::make_shared<std::atomic<int>>(0);
		std::unordered_map<glm::vec3, ChunkContext> chunks;

		const int MAX_CONCURRENCY = 8;
//		std::counting_semaphore<32> semaphore{0};
		Octree(BoundingCube minCube, float chunkSize);
		Octree();
		
		void expand(const WrappedSignedDistanceFunction *function, Transformation model);
		void add(WrappedSignedDistanceFunction *function, const Transformation model, const TexturePainter &painter, float minSize, Simplifier &simplifier, OctreeChangeHandler * changeHandler);
		void del(WrappedSignedDistanceFunction *function, const Transformation model, const TexturePainter &painter, float minSize, Simplifier &simplifier, OctreeChangeHandler * changeHandler);
		NodeOperationResult shape(ShapeContext context, ShapeArgs args, ChunkContext * shapeChunkContext, ChunkContext * chunkContext);
		void shapeChild(ShapeContext context, ShapeArgs args, ShapeChildArgs childArgs, ChunkContext * shapeChunkContext, ChunkContext * chunkContext);
		void iterate(IteratorHandler &handler);
		void iterateFlat(IteratorHandler &handler);

		OctreeNode* getNodeAt(const glm::vec3 &pos, int level, bool simplification);
		OctreeNode* getNodeAt(const glm::vec3 &pos, bool simplification);
		float getSdfAt(const glm::vec3 &pos);
		void handleQuadNodes(OctreeNodeData &data, OctreeNodeTriangleHandler * handler, bool simplification);
		bool hasFinerNode(const OctreeNode *node);
		int getLevelAt(const glm::vec3 &pos, bool simplification);
		int getNeighborLevel(OctreeNodeData &data, bool simplification, int direction);
		OctreeNode * fetch(OctreeNodeData &data, OctreeNode ** out, int i, bool simplification);
		int getMaxLevel(OctreeNode * node, int level);

		int getMaxLevel(BoundingCube &cube);
		int getMaxLevel(OctreeNode *node, BoundingCube &cube, BoundingCube &c, int level);

		void exportOctreeSerialization(OctreeSerialized * octree);
		void exportNodesSerialization(std::vector<OctreeNodeCubeSerialized> * nodes);
	private:
		void buildSDF(SignedDistanceFunction * function, Transformation model, BoundingCube &cube, float * resultSDF, float * existingSDF, ChunkContext * chunkContext);
		void inheritSDF(SignedDistanceFunction * function, Transformation model, BoundingCube &cube, float * resultSDF, float * existingSDF);
	};

class Simplifier {
	float angle;
	float distance;
	bool texturing;
	public:
		Simplifier(float angle, float distance, bool texturing);
		void simplify(Octree * tree, BoundingCube chunkCube, const OctreeNodeData &params);

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

struct alignas(16) InstanceData {
    public:
    glm::mat4 matrix;
    float shift;
	uint animation;
    
	InstanceData() {
        this->matrix = glm::mat4(1.0f);
        this->shift = 0.0f;
		this->animation = 0u;
    }

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
    virtual InstanceGeometry<T> * build(Octree * tree, OctreeNodeData &params) = 0;
};

class MeshGeometryBuilder  : public GeometryBuilder<InstanceData> {
	long * trianglesCount;
	public:
		MeshGeometryBuilder(long * trianglesCount);
		~MeshGeometryBuilder();
		InstanceGeometry<InstanceData> * build(Octree * tree, OctreeNodeData &params) override;
};

class IteratorHandler {
    std::stack<OctreeNodeData> flatData;
    std::stack<StackFrame> stack;
    std::stack<StackFrameOut> stackOut;
	public: 
		virtual bool test(Octree * tree, OctreeNodeData &params) = 0;
		virtual void before(Octree * tree, OctreeNodeData &params) = 0;
		virtual void after(Octree * tree, OctreeNodeData &params) = 0;
		virtual void getOrder(Octree * tree, OctreeNodeData &params, uint8_t * order) = 0;
		void iterate(Octree * tree, OctreeNodeData params);
		void iterateFlatIn(Octree * tree, OctreeNodeData params);
		void iterateFlatOut(Octree * tree, OctreeNodeData params);
		void iterateFlat(Octree * tree, OctreeNodeData params);
};

template <typename T> class InstanceBuilderHandler {
	public:
		virtual void handle(Octree * tree, OctreeNodeData &data, std::vector<T> * instances) = 0;
};

template <typename T> class InstanceBuilder : public IteratorHandler{
	InstanceBuilderHandler<T> * handler;
	std::vector<T> * instances;
	public: 

		InstanceBuilder(InstanceBuilderHandler<T> * handler, std::vector<T> * instances) {
			this->instances = instances;
			this->handler = handler;
		}
		
		void before(Octree * tree, OctreeNodeData &params) {		
			handler->handle(tree, params, instances);
		}
		
		void after(Octree * tree, OctreeNodeData &params) {			
			return;
		}
		
		bool test(Octree * tree, OctreeNodeData &params) {	
			return true;
		}
		
		void getOrder(Octree * tree, OctreeNodeData &params, uint8_t * order){
			for(size_t i = 0 ; i < 8 ; ++i) {
				order[i] = i;
			}
		}
};

class Tesselator : public IteratorHandler, OctreeNodeTriangleHandler{
	Geometry * chunk;
	public:
		Tesselator(Geometry * chunk, long * count);
		void before(Octree * tree, OctreeNodeData &params) override;
		void after(Octree * tree, OctreeNodeData &params) override;
		bool test(Octree * tree, OctreeNodeData &params) override;
		void getOrder(Octree * tree, OctreeNodeData &params, uint8_t * order) override;
		void handle(Vertex &v0, Vertex &v1, Vertex &v2, bool sign) override;

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
		void before(Octree * tree, OctreeNodeData &params) override;
		void after(Octree * tree, OctreeNodeData &params) override;
		bool test(Octree * tree, OctreeNodeData &params) override;
		void getOrder(Octree * tree, OctreeNodeData &params, uint8_t * order) override;

};

#endif
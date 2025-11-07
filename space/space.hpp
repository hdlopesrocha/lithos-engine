#ifndef SPACE_HPP
#define SPACE_HPP
#include <semaphore>
#include <thread>
#include <future>
#include <unordered_set>
#include <utility>
#include <shared_mutex>
#include "../math/math.hpp"
#include "../sdf/SDF.hpp"
#define SQRT_3_OVER_2 0.866025404f
#include "ThreadPool.hpp"
#include "Allocator.hpp"

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
    uint8_t bits;

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
		uint id;
		uint8_t bits;
		float sdf[8];

		OctreeNode();
		OctreeNode(Vertex vertex);
		~OctreeNode();
		OctreeNode * init(Vertex vertex);
		ChildBlock * clear(OctreeAllocator &allocator, OctreeChangeHandler * handler, ChildBlock * block);
		ChildBlock * getBlock(OctreeAllocator &allocator);
		ChildBlock * allocate(OctreeAllocator &allocator);
		ChildBlock * deallocate(OctreeAllocator &allocator, ChildBlock * block);

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

		SpaceType getType();

		void setSDF(float * sdf);
		uint exportSerialization(OctreeAllocator &allocator, std::vector<OctreeNodeCubeSerialized> * nodes, int * leafNodes, BoundingCube cube, BoundingCube chunk, uint level);
		OctreeNode * compress(OctreeAllocator &allocator, BoundingCube * cube, BoundingCube chunk);
};

struct ChildBlock {
	uint children[8]; 

	public:
	ChildBlock();
	ChildBlock * init();
	void clear(OctreeAllocator &allocator, OctreeChangeHandler * handler);

	bool isEmpty();
	void set(uint i, OctreeNode * node, OctreeAllocator &allocator);
	OctreeNode * get(uint i, OctreeAllocator &allocator);
};


struct OctreeNodeData {
	public:
	uint level;
	OctreeNode * node;
	BoundingCube cube;
	void * context;
	float sdf[8];
	OctreeNodeData(uint level, OctreeNode * node, BoundingCube cube, void * context, float * sdf) {
		this->level = level;
		this->node = node;
		this->cube = cube;
		this->context = context;
		SDF::copySDF(sdf, this->sdf);
	}

	OctreeNodeData(const OctreeNodeData &data) {
		this->level = data.level;
		this->node = data.node;
		this->cube = data.cube;
		this->context = data.context;
		SDF::copySDF(data.sdf, this->sdf);
	}


};

class OctreeNodeTriangleHandler {

	public: 
	long * count;
	OctreeNodeTriangleHandler(long * count);
	virtual void handle(Vertex &v0, Vertex &v1, Vertex &v2, bool sign) = 0;
};



class OctreeAllocator {
	public: 
	Allocator<OctreeNode> nodeAllocator = Allocator<OctreeNode>(1024);
	Allocator<ChildBlock> childAllocator = Allocator<ChildBlock>(1024);
	OctreeNode * allocate();
	OctreeNode * get(uint index);
	OctreeNode * deallocate(OctreeNode * node);
	uint getIndex(OctreeNode * node);
    size_t getBlockSize() const;
    size_t getAllocatedBlocksCount() ;

};

struct OctreeNodeFrame {
    OctreeNode* node;
    BoundingCube cube;
	uint level;
	float sdf[8];
	int brushIndex;
	bool interpolated;
	BoundingCube chunkCube;
	OctreeNodeFrame() {
				
	}

	OctreeNodeFrame(const OctreeNodeFrame &t)
		: node(t.node),
		cube(t.cube),
		level(t.level),
		brushIndex(t.brushIndex),
		interpolated(t.interpolated),
		chunkCube(t.chunkCube)
	{
		for (int i = 0; i < 8; ++i) {
			sdf[i] = t.sdf[i];
		}
	}
		
	OctreeNodeFrame(OctreeNode* node, BoundingCube cube, uint level, float * sdf, int brushIndex, bool interpolated, BoundingCube chunkCube) 
		: node(node), cube(cube), level(level), brushIndex(brushIndex), interpolated(interpolated), chunkCube(chunkCube) {
			for(int i = 0; i < 8; ++i) {
				this->sdf[i] = sdf!=NULL ? sdf[i] : INFINITY;
			}	
	}


};

struct NodeOperationResult {
    OctreeNode * node;
	SpaceType shapeType;
	SpaceType resultType;
	bool process;
 	float resultSDF[8];
    float shapeSDF[8];
	bool isSimplified;
	int brushIndex;

	NodeOperationResult() : node(NULL), shapeType(SpaceType::Empty), resultType(SpaceType::Empty), process(false), isSimplified(false), brushIndex(DISCARD_BRUSH_INDEX) {
		SDF::copySDF(NULL, this->resultSDF);	
		SDF::copySDF(NULL, this->shapeSDF);	
	};

    NodeOperationResult(OctreeNode * node, SpaceType shapeType, SpaceType resultType, float * resultSDF, float * shapeSDF, bool process, bool isSimplified, int brushIndex) 
        : node(node), shapeType(shapeType), resultType(resultType), process(process), isSimplified(isSimplified), brushIndex(brushIndex) {
		SDF::copySDF(resultSDF, this->resultSDF);	
		SDF::copySDF(shapeSDF, this->shapeSDF);						
    };
};

struct ShapeArgs {
    float (*operation)(float, float);
    WrappedSignedDistanceFunction * function; 
    const TexturePainter &painter;
    const Transformation model;
    Simplifier &simplifier; 
    OctreeChangeHandler * changeHandler;
	float minSize;

    ShapeArgs(
        float (*operation)(float, float),
        WrappedSignedDistanceFunction * function, 
        const TexturePainter &painter,
        const Transformation model,
        Simplifier &simplifier, 
        OctreeChangeHandler * changeHandler,
	    float minSize
	) :
        operation(operation),
        function(function),
        painter(painter),
        model(model),
        simplifier(simplifier),
        changeHandler(changeHandler),
		minSize(minSize)
		 {

            
        };


};


class ThreadContext {
	public:
	std::unordered_map<glm::vec3, float> shapeSdfCache;
	std::unordered_map<glm::vec4, OctreeNode*> nodeCache;
    std::shared_mutex mutex;
	BoundingCube cube;
	
	ThreadContext(BoundingCube cube) : cube(cube) {
		shapeSdfCache.clear();
		nodeCache.clear();
	}
};



class Octree: public BoundingCube {
	using BoundingCube::BoundingCube;
	public: 
		float chunkSize;
		OctreeNode * root;
		OctreeAllocator * allocator;
		int threadsCreated = 0;
		std::shared_ptr<std::atomic<int>> shapeCounter = std::make_shared<std::atomic<int>>(0);
		std::unordered_map<glm::vec3, ThreadContext> chunks;
		ThreadPool threadPool = ThreadPool(std::thread::hardware_concurrency());
		std::mutex mutex;
		Octree(BoundingCube mxnCube, float chunkSize);
		Octree();
		
		void expand(const ShapeArgs &args);
		void add(WrappedSignedDistanceFunction *function, const Transformation model, const TexturePainter &painter, float minSize, Simplifier &simplifier, OctreeChangeHandler * changeHandler);
		void del(WrappedSignedDistanceFunction *function, const Transformation model, const TexturePainter &painter, float minSize, Simplifier &simplifier, OctreeChangeHandler * changeHandler);
		NodeOperationResult shape(OctreeNodeFrame frame, const ShapeArgs &args, ThreadContext * threadContext, std::string coords);
		void iterate(IteratorHandler &handler);
		void iterateFlat(IteratorHandler &handler);

		OctreeNode* getNodeAt(const glm::vec3 &pos, int level, bool simplification);
		OctreeNode* getNodeAt(const glm::vec3 &pos, bool simplification);
		float getSdfAt(const glm::vec3 &pos);
		void handleQuadNodes(const BoundingCube &cube, uint level, const float sdf[8], std::vector<OctreeNodeTriangleHandler*> * handlers, bool simplification, ThreadContext * context);
		OctreeNode * fetch(const BoundingCube &cube, uint level, OctreeNode ** out, int i, bool simplification, ThreadContext * context);
		int getMaxLevel(OctreeNode * node, int level);

		uint getMaxLevel(BoundingCube &cube);
		uint getMaxLevel(OctreeNode *node, BoundingCube &cube, BoundingCube &c, uint level);
		bool isChunkNode(float length) const;
		bool isThreadNode(float length, float minSize, int threadSize) const;
		void exportOctreeSerialization(OctreeSerialized * octree);
		void exportNodesSerialization(std::vector<OctreeNodeCubeSerialized> * nodes);
	private:
		void buildSDF(const ShapeArgs &args, BoundingCube &cube, float shapeSDF[8], float resultSDF[8], float existingResultSDF[8], ThreadContext * threadContext) const;
		float evaluateSDF(const ShapeArgs &args, std::unordered_map<glm::vec3, float> * threadContext, glm::vec3 p) const;
	};

class Simplifier {
	float angle;
	float distance;
	bool texturing;
	public:
		Simplifier(float angle, float distance, bool texturing);
		std::pair<bool,int> simplify(const BoundingCube chunkCube, const BoundingCube cube, const float * sdf, NodeOperationResult * children);
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
    glm::vec4 sdf1;     // 16 bytes
    glm::vec4 sdf2;     // 16 bytes
	glm::mat4 matrix;
	int brushIndex;

    DebugInstanceData(glm::mat4 matrix, float * sdf, int brushIndex) {
		sdf1 = glm::vec4(sdf[0], sdf[1], sdf[2], sdf[3]);
        sdf2 = glm::vec4(sdf[4], sdf[5], sdf[6], sdf[7]);
        this->matrix = matrix;
		this->brushIndex = brushIndex;
    }
};

template <typename T> class GeometryBuilder {
    public:
    virtual InstanceGeometry<T> * build(Octree * tree, OctreeNodeData &params, ThreadContext * context) = 0;
};

class IteratorHandler {
    std::stack<OctreeNodeData> flatData;
    std::stack<StackFrame> stack;
    std::stack<StackFrameOut> stackOut;
	public: 
		virtual bool test(Octree * tree, OctreeNodeData *params) = 0;
		virtual void before(Octree * tree, OctreeNodeData *params) = 0;
		virtual void after(Octree * tree, OctreeNodeData *params) = 0;
		virtual void getOrder(Octree * tree, OctreeNodeData *params, uint8_t * order) = 0;
		void iterate(Octree * tree, OctreeNodeData *params);
		void iterateFlatIn(Octree * tree, OctreeNodeData *params);
		void iterateFlatOut(Octree * tree, OctreeNodeData *params);
		void iterateFlat(Octree * tree, OctreeNodeData *params);
};

template <typename T> class InstanceBuilderHandler {
	public:
		virtual void handle(Octree * tree, OctreeNodeData &data, std::vector<T> * instances, ThreadContext * context) = 0;
};

template <typename T> class InstanceBuilder : public IteratorHandler{
	InstanceBuilderHandler<T> * handler;
	std::vector<T> * instances;
	ThreadContext * context;
	public: 

		InstanceBuilder(InstanceBuilderHandler<T> * handler, std::vector<T> * instances, ThreadContext * context){
			this->instances = instances;
			this->handler = handler;
			this->context = context;
		}
		
		void before(Octree * tree, OctreeNodeData *params) {
		}
		
		void after(Octree * tree, OctreeNodeData *params) {	
			handler->handle(tree, *params, instances, context);
			return;
		}
		
		bool test(Octree * tree, OctreeNodeData *params) {	
			return params->node != NULL;
		}
		
		void getOrder(Octree * tree, OctreeNodeData *params, uint8_t * order) {
			for(size_t i = 0 ; i < 8 ; ++i) {
				order[i] = i;
			}
		}
};

class Tesselator : public OctreeNodeTriangleHandler{
	ThreadContext * context;

	public:
		Geometry * geometry;
		Tesselator(long * count, ThreadContext * context);
		void handle(Vertex &v0, Vertex &v1, Vertex &v2, bool sign) override;

};

class Processor : public IteratorHandler {
	ThreadContext * context;
	std::vector<OctreeNodeTriangleHandler*> * handlers;
	public:
		Processor(long * count, ThreadContext * context, std::vector<OctreeNodeTriangleHandler*> * handlers);
		void before(Octree * tree, OctreeNodeData *params) override;
		void after(Octree * tree, OctreeNodeData *params) override;
		bool test(Octree * tree, OctreeNodeData *params) override;
		void getOrder(Octree * tree, OctreeNodeData *params, uint8_t * order) override;
		void virtualize(Octree * tree, const BoundingCube &cube, float * sdf, uint level, uint levels);

};



class OctreeFile {
	Octree * tree;
    std::string filename;
    public: 
		OctreeFile(Octree * tree, std::string filename);
        void save(std::string baseFolder, float chunkSize);
        void load(std::string baseFolder, float chunkSize);
		AbstractBoundingBox& getBox();
		OctreeNode * loadRecursive(int i, std::vector<OctreeNodeSerialized> * nodes, float chunkSize, std::string filename, BoundingCube cube, std::string baseFolder);
		uint saveRecursive(OctreeNode * node, std::vector<OctreeNodeSerialized> * nodes, float chunkSize, std::string filename, BoundingCube cube, std::string baseFolder);

};

class OctreeNodeFile {
	OctreeNode * node;
    std::string filename;
	Octree * tree;
    public: 
		OctreeNodeFile(Octree * tree, OctreeNode * node, std::string filename);
        void save(std::string baseFolder);
        void load(std::string baseFolder, BoundingCube &cube);
		OctreeNode * loadRecursive(OctreeNode * node, int i, BoundingCube &cube, std::vector<OctreeNodeSerialized> * nodes);
		uint saveRecursive(OctreeNode * node, std::vector<OctreeNodeSerialized> * nodes);
};


class OctreeVisibilityChecker : public IteratorHandler{
	Frustum frustum;
	public:
		glm::vec3 sortPosition;
		std::vector<OctreeNodeData> visibleNodes;
		OctreeVisibilityChecker();
		void update(glm::mat4 m);
		void before(Octree * tree, OctreeNodeData *params) override;
		void after(Octree * tree, OctreeNodeData *params) override;
		bool test(Octree * tree, OctreeNodeData *params) override;
		void getOrder(Octree * tree, OctreeNodeData *params, uint8_t * order) override;

};

#endif
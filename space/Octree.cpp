#include "space.hpp"

//      6-----7
//     /|    /|
//    4z+---5 |
//    | 2y--+-3
//    |/    |/
//    0-----1x

static std::vector<glm::ivec4> TESSELATION_ORDERS;
static std::vector<glm::ivec2> TESSELATION_EDGES;
static bool initialized = false;

static void initialize() {
    if(!initialized) {
        TESSELATION_ORDERS.push_back(glm::ivec4(0,1,3,2));TESSELATION_EDGES.push_back(SDF_EDGES[11]);
        TESSELATION_ORDERS.push_back(glm::ivec4(0,2,6,4));TESSELATION_EDGES.push_back(SDF_EDGES[6]);
        TESSELATION_ORDERS.push_back(glm::ivec4(0,4,5,1));TESSELATION_EDGES.push_back(SDF_EDGES[5]);
        initialized = true;
    }
}

Octree::Octree(BoundingCube minCube, float chunkSize) : BoundingCube(minCube), allocator(new OctreeAllocator()) {
    this->chunkSize = chunkSize;
	this->root = allocator->allocate()->init(glm::vec3(minCube.getCenter()));
	initialize();
}

Octree::Octree() : Octree(glm::vec3(0.0f), 1.0f) {
    this->chunkSize = 1.0f;
    this->root = NULL;
    initialize();
}

int getNodeIndex(const glm::vec3 &vec, const BoundingCube &cube) {
	glm::vec3 c = cube.getCenter();
    return (vec.x >= c.x ? 4 : 0) + (vec.y >= c.y ? 2 : 0) + (vec.z >= c.z ? 1 : 0);
}




OctreeNodeLevel Octree::getNodeAt(const glm::vec3 &pos, int level, bool simplification) const{
    OctreeNode * candidate = root;
    OctreeNode* node = candidate;
    BoundingCube cube = *this;
    uint currentLevel = 0;
	if(!contains(pos)) {
		return OctreeNodeLevel(NULL, 0);
	}
    while (candidate != NULL && level-- > 0 ) {
        if (simplification && node->isSimplified()) {
            break;
        }
        int i = getNodeIndex(pos, cube);
        cube = cube.getChild(i);
        ChildBlock * block = node->getBlock(*allocator);
        candidate = block != NULL ? block->get(i, *allocator) : NULL;
        if(candidate != NULL) {
            node = candidate;
            ++currentLevel;
        }
    }
    return OctreeNodeLevel(node, currentLevel);
}

OctreeNode* Octree::getNodeAt(const glm::vec3 &pos, bool simplification) const {
    OctreeNode * candidate = root;
    OctreeNode* node = candidate;
    BoundingCube cube = *this;
	if(!contains(pos)) {
		return NULL;
	}
    while (candidate != NULL) {
        if (simplification && node->isSimplified()) {
            break;
        }
        int i = getNodeIndex(pos, cube);
        cube = cube.getChild(i);
        ChildBlock * block = node->getBlock(*allocator);
        candidate = block != NULL ? block->get(i, *allocator) : NULL;
        if(candidate != NULL) {
            node = candidate;
        }
    }
    return node;
}

float Octree::getSdfAt(const glm::vec3 &pos) {
    OctreeNode * candidate = root;
    OctreeNode * node = candidate;
    BoundingCube candidateCube = *this;
    BoundingCube nodeCube = candidateCube;

	if(!contains(pos)) {
		return INFINITY;
	}
    while (candidate) {
        node = candidate;
        nodeCube = candidateCube;
        int i = getNodeIndex(pos, candidateCube);
        candidateCube = nodeCube.getChild(i);
        ChildBlock * block = node->getBlock(*allocator);
        candidate = block != NULL ? block->get(i, *allocator) : NULL;
    }

    if(node) {
        return SDF::interpolate(node->sdf, pos, nodeCube);
    }
    std::cerr << "Not interpolated" << std::endl;
    return INFINITY;
}
void Octree::iterateBorder(
            const OctreeNode * from,
            const BoundingCube &fromCube,
            const float fromSDF[8],
            const uint fromLevel,

            const OctreeNode *to,
            const BoundingCube &toCube,
            const float toSDF[8],
            const uint toLevel,
            bool &nodeIterated, 
            const IterateBorderHandler &func) const
{
    if (!to) return;

    // ----------------------
    // LEAF / SIMPLIFIED CASE
    // ----------------------
    if (to->isSimplified()) {

        // CASE A: toCube is same size or finer than fromCube
        // -> build pseudo as a clipped subregion of toCube (use toSDF)
        if (toCube.getLengthX() < fromCube.getLengthX()) {
            BoundingCube toCubeShifted = toCube;
            int axis = -1;

            if (toCube.getMinX() <= fromCube.getMaxX() && toCube.getMaxX() > fromCube.getMaxX())
                axis = 0;
            else if (toCube.getMinY() <= fromCube.getMaxY() && toCube.getMaxY() > fromCube.getMaxY())
                axis = 1;
            else if (toCube.getMinZ() <= fromCube.getMaxZ() && toCube.getMaxZ() > fromCube.getMaxZ())
                axis = 2;

            if (axis != -1) {
                BoundingCube toCubeShifted = toCube;
                if (axis == 0) toCubeShifted.setMaxX(fromCube.getMaxX());
                if (axis == 1) toCubeShifted.setMaxY(fromCube.getMaxY());
                if (axis == 2) toCubeShifted.setMaxZ(fromCube.getMaxZ());

                if (fromCube.intersects(toCubeShifted)) {
                    float toSdfShifted[8];
                    for (uint i = 0; i < 8; ++i) {
                        toSdfShifted[i] = SDF::interpolate(toSDF, toCubeShifted.getCorner(i), toCube);
                    }
                    func(toCubeShifted, toSdfShifted, toLevel);
                }
            }
        }
        // CASE B: toCube is coarser/larger than fromCube
        // -> build pseudo as a clipped subregion of fromCube (use fromSDF)
        else {
            if(to->getType() == SpaceType::Surface) {
                if(!nodeIterated) {
                    nodeIterated = true;
                    func(fromCube, fromSDF, fromLevel);
                }
            }
        }

        return;
    }

    // ----------------------
    // INTERNAL NODE: recurse into children of `to`
    // ----------------------
    OctreeNode * children[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    to->getChildren(*allocator, children);

    // decide a threshold for treating `to` as "similar size" to `from`
    // (kept your original heuristic, but you can tune or replace it)

    for (uint i = 0; i < 8; ++i) {
        OctreeNode * to = children[i];
        if (to != NULL && to->getType() == SpaceType::Surface) {
            BoundingCube childCube = toCube.getChild(i);
            int axis = -1;

            if (childCube.getMinX() <= fromCube.getMaxX() && childCube.getMaxX() > fromCube.getMaxX())
                axis = 0;
            else if (childCube.getMinY() <= fromCube.getMaxY() && childCube.getMaxY() > fromCube.getMaxY())
                axis = 1;
            else if (childCube.getMinZ() <= fromCube.getMaxZ() && childCube.getMaxZ() > fromCube.getMaxZ())
                axis = 2;

            if (axis != -1 && fromCube.intersects(childCube)) {
                iterateBorder(from, fromCube, fromSDF, fromLevel, to, childCube, to->sdf, toLevel + 1, nodeIterated, func);
            }
        }
    }
}


OctreeNodeLevel Octree::fetch(glm::vec3 pos, uint level, bool simplification, ThreadContext * context) const {
    glm::vec4 key = glm::vec4(pos, level);
    if(context->nodeCache.find(key) != context->nodeCache.end()) {
        return context->nodeCache[key];
    } else {
        OctreeNodeLevel nodeLevel = getNodeAt(pos, level, simplification);
        context->nodeCache[key] = nodeLevel;
        return nodeLevel;
    }
}

template <typename T, std::size_t N> 
bool allDifferent(const T (&arr)[N]) {
    for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = i + 1; j < N; ++j) {
            if (arr[i] == arr[j]) return false;
        }
    }
    return true;
}

template <typename T, typename... Args>
bool allDifferent(const T& first, const Args&... args) {
    std::array<T, sizeof...(args) + 1> arr { first, args... };

    for (std::size_t i = 0; i < arr.size(); ++i) {
        for (std::size_t j = i + 1; j < arr.size(); ++j) {
            if (arr[i] == arr[j]) return false;
        }
    }
    return true;
}

void Octree::handleQuadNodes(const BoundingCube &cube, uint level, const float sdf[8], std::vector<OctreeNodeTriangleHandler*> * handlers, bool simplification, ThreadContext * context) const {
    OctreeNodeLevel neighbors[8] = {
        OctreeNodeLevel(),OctreeNodeLevel(),OctreeNodeLevel(),OctreeNodeLevel(),
        OctreeNodeLevel(),OctreeNodeLevel(),OctreeNodeLevel(),OctreeNodeLevel()
    };
    for(size_t k =0 ; k < TESSELATION_EDGES.size(); ++k) {
		glm::ivec2 &edge = TESSELATION_EDGES[k];
		bool sign0 = sdf[edge[0]] < 0.0f;
		bool sign1 = sdf[edge[1]] < 0.0f;

		if(sign0 != sign1) {
			glm::ivec4 &quad = TESSELATION_ORDERS[k];
            Vertex vertices[4] = { Vertex(), Vertex(), Vertex(), Vertex() };
			for(uint i =0; i < 4 ; ++i) {
                uint neighborIndex = quad[i];
				OctreeNodeLevel * neighbor = &neighbors[neighborIndex];
                if(neighbor->node == NULL) {
                    glm::vec3 pos = cube.getCenter() + cube.getLength() * Octree::getShift(neighborIndex);
                    *neighbor = fetch(pos, level, simplification, context);              
                }
                OctreeNode * childNode = neighbor->node;
                if(childNode != NULL && childNode->getType() == SpaceType::Surface) {
                    vertices[i] = childNode->vertex;
                } else {
                    vertices[i].brushIndex = DISCARD_BRUSH_INDEX;
                }
			}
	
            if(allDifferent(vertices[0], vertices[2], vertices[1])) {
                for(auto handler : *handlers) {
                    handler->handle(vertices[0], vertices[2], vertices[1], sign1);
                }
			}
            if(allDifferent(vertices[0], vertices[3], vertices[2])) {
                for(auto handler : *handlers) {
                    handler->handle(vertices[0], vertices[3], vertices[2], sign1);
                }
            }
		}
	}
}

float Octree::evaluateSDF(const ShapeArgs &args, std::unordered_map<glm::vec3, float> * cache, glm::vec3 p) const {
    if(cache->find(p) != cache->end()) {
       return (*cache)[p];
    } else {
        float d = args.function->distance(p, args.model);
        (*cache)[p] = d;
        return d;
    }
}

void Octree::buildSDF(const ShapeArgs &args, BoundingCube &cube, float shapeSDF[8], float resultSDF[8], float existingResultSDF[8], ThreadContext * threadContext) const {
    const glm::vec3 min = cube.getMin();
    const glm::vec3 length = cube.getLength();
    std::unordered_map<glm::vec3, float> * shapeSdfCache = &threadContext->shapeSdfCache;

    for (uint i = 0; i < 8; ++i) {
        if(shapeSDF[i] == INFINITY) {
            shapeSDF[i] = evaluateSDF(args, shapeSdfCache, min + length * Octree::getShift(i));
        }
        if(resultSDF[i] == INFINITY) {
            resultSDF[i] = args.operation(existingResultSDF[i], shapeSDF[i]);
        }
    }
}

void Octree::expand(const ShapeArgs &args) {
    while (!args.function->isContained(*this, args.model, args.minSize)) {
        glm::vec3 point = args.function->getCenter(args.model);
        unsigned int i = getNodeIndex(point, *this) ^ 0x7;

        setMin(getMin() - Octree::getShift(i) * getLengthX());
        setLength(getLengthX()*2);

        OctreeNode* oldRoot = root;
        OctreeNode* newRoot = allocator->allocate()->init(getCenter());
        ChildBlock* newBlock = newRoot->allocate(*allocator)->init();

        if (oldRoot != NULL) {
            ChildBlock* oldBlock = oldRoot->getBlock(*allocator);
            bool emptyNode = oldRoot->getType() == SpaceType::Empty;
            bool emptyBlock = (oldBlock == NULL || oldBlock->isEmpty());

            if (emptyNode && emptyBlock) {
                if (oldBlock != NULL) {
                    oldBlock = oldRoot->clear(*allocator, args.changeHandler, oldBlock);
                }
                oldRoot = allocator->deallocate(oldRoot);
            }
        }
        if (newRoot == oldRoot) {
            throw std::runtime_error("Infinite recursion!");
        }

        if (oldRoot != NULL) {
            newBlock->set(i, oldRoot, *allocator);
        }
        root = newRoot;
    }
}

void Octree::add(
        WrappedSignedDistanceFunction *function, 
        const Transformation model, 
        glm::vec4 translate,
        glm::vec4 scale,
        const TexturePainter &painter,
        float minSize, 
        Simplifier &simplifier, 
        OctreeChangeHandler * changeHandler
    ) {
    threadsCreated = 0;
    *shapeCounter = 0;
    ShapeArgs args = ShapeArgs(SDF::opUnion, function, painter, model, translate, scale, simplifier, changeHandler, minSize);	
  	expand(args);
    OctreeNodeFrame frame = OctreeNodeFrame(root, *this, 0, root->sdf, DISCARD_BRUSH_INDEX, false, *this);
    ThreadContext localChunkContext = ThreadContext(*this);
    shape(frame, args, &localChunkContext);
    std::cout << "\t\tOctree::add Ok! threads=" << threadsCreated << ", works=" << *shapeCounter << std::endl; 
}

void Octree::del(
        WrappedSignedDistanceFunction * function, 
        const Transformation model, 
        glm::vec4 translate,
        glm::vec4 scale,
        const TexturePainter &painter,
        float minSize, Simplifier &simplifier, OctreeChangeHandler  * changeHandler
    ) {
    threadsCreated = 0;
    *shapeCounter = 0;
    ShapeArgs args = ShapeArgs(SDF::opSubtraction, function, painter, model, translate, scale, simplifier, changeHandler, minSize);
    OctreeNodeFrame frame = OctreeNodeFrame(root, *this, 0, root->sdf, DISCARD_BRUSH_INDEX, false, *this);
    ThreadContext localChunkContext = ThreadContext(*this);
    shape(frame, args, &localChunkContext);
    std::cout << "\t\tOctree::del Ok! threads=" << threadsCreated << ", works=" << *shapeCounter << std::endl; 
}

SpaceType childToParent(bool childSolid, bool childEmpty) {
    if(childSolid) {
        return SpaceType::Solid;
    } else if(childEmpty) {
        return SpaceType::Empty;
    } else {
        return SpaceType::Surface;
    }
}

bool Octree::isChunkNode(float length) const {
    return chunkSize*0.5f < length && length <= chunkSize;
}

bool Octree::isThreadNode(float length, float minSize, int threadSize) const {
    return minSize*threadSize < length;
}

NodeOperationResult Octree::shape(OctreeNodeFrame frame, const ShapeArgs &args, ThreadContext * threadContext) {    
    ContainmentType check = args.function->check(frame.cube, args.model, args.minSize);
    OctreeNode * node = frame.node;
    bool process = true;

    if(check == ContainmentType::Disjoint) {
        process = false;
        SpaceType resultType = node ? node->getType() : SDF::eval(frame.sdf);
        return NodeOperationResult(node, SpaceType::Empty, resultType, frame.sdf, INFINITY_ARRAY, process, node ? node->isSimplified() : true, DISCARD_BRUSH_INDEX);  // Skip this node
    }
    float length = frame.cube.getLengthX();
    bool isChunk = isChunkNode(length);
    bool isLeaf = length <= args.minSize;
    if(node != NULL && !node->isLeaf()) {
        isLeaf = false;
    }

    NodeOperationResult childResult[8];
    std::vector<std::thread> threads;
    threads.reserve(8);
    if (!isLeaf) {
        bool isChildThread = isThreadNode(length*0.5f, args.minSize, 16);
        bool isChildChunk = isChunkNode(length*0.5f);

        OctreeNode * children[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
        if(node != NULL) {
            node->getChildren(*allocator, children);
        }
        // --------------------------------
        // Iterate nodes and create threads
        // --------------------------------

        for (uint i = 0; i < 8; ++i) {
            OctreeNode * child = children[i];
            if(node!=NULL && child == node) {
		        throw std::runtime_error("Infinite loop " + std::to_string((long)child) + " " + std::to_string((long)node));
            }
            
            float childSDF[8] = {INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY};
            int childBrushIndex = node != NULL ? node->vertex.brushIndex : frame.brushIndex;
            bool isChildInterpolated = frame.interpolated;

            if(child != NULL) {
                childBrushIndex = child->vertex.brushIndex;
                SDF::copySDF(child->sdf, childSDF);
            } else {
                isChildInterpolated = true;
                SDF::getChildSDF(frame.sdf, i, childSDF);
            }
            BoundingCube childCube = frame.cube.getChild(i);
            OctreeNodeFrame childFrame = OctreeNodeFrame(
                child, 
                childCube, 
                frame.level + 1, 
                childSDF,
                childBrushIndex,
                isChildInterpolated,
                isChildChunk ? childCube : frame.chunkCube
            );

            if(isChildThread) {
                ++threadsCreated;
                NodeOperationResult * result = &childResult[i];
                threads.emplace_back([this, childFrame, args, result]() {
                   ThreadContext localThreadContext(childFrame.cube);
                   *result = shape(childFrame, args, &localThreadContext);
                });
            } else {
                childResult[i] = shape(childFrame, args, threadContext);
            }
            
            (*shapeCounter)++;
        }
    }

    // ------------------------------
    // Synchronize threads if created
    // ------------------------------

    for(std::thread &t : threads) {
        if(t.joinable()) {
            t.join();
        }
    }
 
    // ------------------------------
    // Inherit SDFs from children
    // ------------------------------

    float shapeSDF[8] = {INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY};
    float resultSDF[8] = {INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY};
    bool childResultSolid = true;
    bool childResultEmpty = true;
    bool childShapeSolid = true;
    bool childShapeEmpty = true;
    bool childSimplified = true;
    bool isSimplified = isLeaf;
    int brushIndex = frame.brushIndex;

    if(!isLeaf) {
        for(uint i = 0; i < 8; ++i) {
            NodeOperationResult & result = childResult[i];   

            childResultEmpty &= result.resultType == SpaceType::Empty;
            childResultSolid &= result.resultType == SpaceType::Solid;
            childShapeEmpty &= result.shapeType == SpaceType::Empty;
            childShapeSolid &= result.shapeType == SpaceType::Solid;
            childSimplified &= result.isSimplified;
            if(result.process) {
                resultSDF[i] = result.resultSDF[i];
                shapeSDF[i] = result.shapeSDF[i];
            } 
        }
    }

    // ------------------------------
    // Build SDFs based on inheritance/execution
    // ------------------------------
    buildSDF(args, frame.cube, shapeSDF, resultSDF, frame.sdf, threadContext);
    
    SpaceType shapeType = isLeaf ? SDF::eval(shapeSDF) : childToParent(childShapeSolid, childShapeEmpty);
    SpaceType resultType = isLeaf ? SDF::eval(resultSDF) : childToParent(childResultSolid, childResultEmpty);

    if(shapeType == SpaceType::Empty && !frame.interpolated) {
        // Do nothing
        process = false;
    }
    else if(resultType == SpaceType::Surface) {
        // ------------------------------
        // Created nodes if the shape is not Empty
        // ------------------------------     
        if(node == NULL) {
            node = allocator->allocate()->init(Vertex(frame.cube.getCenter()));   
        }

        if(node!= NULL) {
            node->vertex.position = glm::vec4(SDF::getAveragePosition(resultSDF, frame.cube) ,0.0f);
            node->vertex.normal = glm::vec4(SDF::getNormalFromPosition(resultSDF, frame.cube, node->vertex.position), 0.0f);        
            
            // ------------------------------
            // Simplification & Painting
            // ------------------------------
            if(isLeaf) {
                if(shapeType != SpaceType::Empty) {
                    brushIndex = args.painter.paint(node->vertex, args.translate, args.scale);
                }        
            } else {
                if(childSimplified && !isChunk) {
                    std::pair<bool,int> simplificationResult = args.simplifier.simplify(frame.chunkCube, frame.cube, resultSDF, childResult);
                    isSimplified = simplificationResult.first;
                    brushIndex = simplificationResult.second;
                }
            }

            node->vertex.brushIndex = brushIndex;

            if(!isLeaf) {
                // ------------------------------
                // Created at least one solid child
                // ------------------------------
                bool isChildChunk = isChunkNode(length*0.5f);
                uint childNodes[8] = {UINT_MAX,UINT_MAX,UINT_MAX,UINT_MAX,UINT_MAX,UINT_MAX,UINT_MAX,UINT_MAX};
                for(uint i =0 ; i < 8 ; ++i) {
                    NodeOperationResult & child = childResult[i];
                    OctreeNode * childNode = child.node;

                    if(child.process) {
                        if(child.resultType != SpaceType::Surface) {
                            BoundingCube childCube = frame.cube.getChild(i);
                            bool childIsLeaf = length *0.5f <= args.minSize;
                           
                            if(childNode == NULL) {
                                childNode = allocator->allocate()->init(Vertex(childCube.getCenter()));
                                childResult[i].node = childNode;
                            }
                            childNode->setType(child.resultType);
                            childNode->setSDF(child.resultSDF);
                            childNode->setLeaf(childIsLeaf);
                            childNode->setSimplified(childIsLeaf);
                            childNode->setChunk(isChildChunk);
                            childNode->setDirty(true);
                        }
                    }
                    childNodes[i] = allocator->nodeAllocator.getIndex(childNode);
                    if(frame.node != NULL && childNode == node) {
                        throw std::runtime_error("Infinite recursion! " + std::to_string((long) childNode) + " " + std::to_string((long)node) );
                    }                
                }
                node->setChildren(*allocator, childNodes);
            }
            if(isChunk && args.changeHandler != NULL) {
                args.changeHandler->update(node);
            }
        }
    } else if(resultType != SpaceType::Surface) {
        isSimplified = true;
        if(node != NULL && isChunk && args.changeHandler != NULL) {
            args.changeHandler->erase(node);
        }

        // ------------------------------
        // Delete nodes if result is Empty
        // ------------------------------
        ChildBlock * block = node ? node->getBlock(*allocator) : NULL;
        if(block) {
            node->clear(*allocator, args.changeHandler, block);
        } 
        /*if(resultType == SpaceType::Empty) {
            node = node ? allocator->deallocate(node) : NULL;
        }
        */
    }

    if(node!= NULL && process) {
        node->setSDF(resultSDF);
        node->setType(resultType);
        node->setChunk(isChunk);
        node->setDirty(true);
        node->setSimplified(isSimplified);
        node->setLeaf(isLeaf);
    }

    return NodeOperationResult(node, shapeType, resultType, resultSDF, shapeSDF, process, isSimplified, brushIndex);
}


void Octree::iterate(IteratorHandler &handler) {
    OctreeNodeData data(0, root, *this, ContainmentType::Intersects, NULL, root->sdf);
	handler.iterate(*this, data);
}

void Octree::iterateFlat(IteratorHandler &handler) {
    OctreeNodeData data(0, root, *this, ContainmentType::Intersects, NULL, root->sdf);
    handler.iterateFlatIn(*this, data);
}

void Octree::iterateParallel(IteratorHandler &handler) {
    OctreeNodeData data(0, root, *this, ContainmentType::Intersects, NULL, root->sdf);
    handler.iterateBFS(*this, data);
    //handler.iterateParallelBFS(*this, data, threadPool);
}

void Octree::exportOctreeSerialization(OctreeSerialized * node) {
    std::cout << "exportOctreeSerialization()" << std::endl;
    for(int i = 0; i < 3; ++i) {
        node->min[i] = this->min[i];
        std::cout << "\tmin["<<std::to_string(i) <<"]: " << std::to_string(node->min[i]) << std::endl;
    }
    node->length = this->length;
    node->chunkSize = this->chunkSize;

    std::cout << "\tlength: " << std::to_string(node->length) << std::endl;
    std::cout << "\tchunkSize: " << std::to_string(node->chunkSize) << std::endl;
}

void Octree::exportNodesSerialization(std::vector<OctreeNodeCubeSerialized> * nodes) {
	std::cout << "exportNodesSerialization()" << std::endl;
    nodes->clear();
    nodes->reserve(10000000);
    int leafNodes = 0;
    root->exportSerialization(*allocator, nodes, &leafNodes, *this, *this, 0u);
	std::cout << "exportNodesSerialization Ok!" << std::endl;
}


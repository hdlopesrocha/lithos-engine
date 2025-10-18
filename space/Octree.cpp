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
        TESSELATION_ORDERS.push_back(glm::ivec4(0,1,3,2));TESSELATION_EDGES.push_back(glm::ivec2(3,7));
        TESSELATION_ORDERS.push_back(glm::ivec4(0,2,6,4));TESSELATION_EDGES.push_back(glm::ivec2(6,7));
        TESSELATION_ORDERS.push_back(glm::ivec4(0,4,5,1));TESSELATION_EDGES.push_back(glm::ivec2(5,7));
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

OctreeNode* Octree::getNodeAt(const glm::vec3 &pos, int level, bool simplification) {
    OctreeNode * candidate = root;
    OctreeNode* node = candidate;
    BoundingCube cube = *this;
	if(!contains(pos)) {
		return NULL;
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
        }
    }
    return node;
}

OctreeNode* Octree::getNodeAt(const glm::vec3 &pos, bool simplification) {
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
        candidate = block ? block->get(i, *allocator) : NULL;
    }

    if(node) {
        return SDF::interpolate(node->sdf, pos, nodeCube);
    }
    std::cerr << "Not interpolated" << std::endl;
    return INFINITY;
}

uint Octree::getMaxLevel(BoundingCube &cube) {
    return getMaxLevel(this->root, cube, *this, 0);
}


uint Octree::getMaxLevel(OctreeNode *node, BoundingCube &cube, BoundingCube &nodeCube, uint level) {
    uint l = level;
    if(!node->isSimplified()) {
        bool isSize = nodeCube.getLengthX() <= cube.getLengthX()*2.0f;
        ChildBlock * block = node->getBlock(*allocator);
        if(block != NULL) {
            for(int i=0; i < 8; ++i) {
                BoundingCube childCube = nodeCube.getChild(i); 
                    // Neighbor conditions:
                bool isAfter = !isSize || (isSize && (
                    childCube.getMinX() == cube.getMaxX() ||
                    childCube.getMinY() == cube.getMaxY() ||
                    childCube.getMinZ() == cube.getMaxZ()
                ));
                if(isAfter) {
                    bool isNeighbor = !cube.contains(childCube) && cube.intersects(childCube);
                    if (isNeighbor) {
                        OctreeNode * childNode = block->get(i, *allocator);
                        if(childNode!=NULL) {
                            l = glm::max(l, getMaxLevel(childNode, cube, childCube, level + 1));
                        }
                    }
                }
                
            }
        }
    }
    return l;
}

OctreeNode * Octree::fetch(const OctreeNodeData &data, OctreeNode ** out, int i, bool simplification, ThreadContext * context) {
    if(out[i] == NULL) {
        glm::vec3 pos = data.cube.getCenter() + data.cube.getLength() * Octree::getShift(i);
        glm::vec4 key = glm::vec4(pos, data.level);
        OctreeNode * node;
        if(context->nodeCache.find(key) != context->nodeCache.end()) {
            node = context->nodeCache[key];
        } else {
            node = getNodeAt(pos, data.level, simplification);
            context->nodeCache[key] = node;
        }
        out[i] = node;
    }
    return out[i];
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

void Octree::handleQuadNodes(const OctreeNodeData &data, const float * sdf, std::vector<OctreeNodeTriangleHandler*> * handlers, bool simplification, ThreadContext * context) {
    OctreeNode * neighbors[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	
    for(size_t k =0 ; k < TESSELATION_EDGES.size(); ++k) {
		glm::ivec2 edge = TESSELATION_EDGES[k];
		bool sign0 = sdf[edge[0]] < 0.0f;
		bool sign1 = sdf[edge[1]] < 0.0f;

		if(sign0 != sign1) {
			glm::ivec4 quad = TESSELATION_ORDERS[k];
            Vertex vertices[4] = { Vertex(), Vertex(), Vertex(), Vertex() };
			for(int i =0; i < 4 ; ++i) {
				OctreeNode * childNode = fetch(data, neighbors, quad[i], simplification, context);
                if(childNode != NULL && !childNode->isSolid() && !childNode->isEmpty()) {
                    vertices[i] = childNode->vertex;
                } else {
                    vertices[i].brushIndex = DISCARD_BRUSH_INDEX;
                }
			}
	
            if(allDifferent(vertices[0], vertices[2], vertices[1])) {
                for(auto handler : *handlers) {
                    handler->handle(data, vertices[0], vertices[2], vertices[1], sign1);
                }
			}
            if(allDifferent(vertices[0], vertices[3], vertices[2])) {
                for(auto handler : *handlers) {
                    handler->handle(data, vertices[0], vertices[3], vertices[2], sign1);
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

void Octree::buildSDF(const ShapeArgs &args, BoundingCube &cube, float * shapeSDF, float * resultSDF, float * existingResultSDF, ThreadContext * threadContext) const {
    const glm::vec3 min = cube.getMin();
    const glm::vec3 length = cube.getLength();
    std::unordered_map<glm::vec3, float> * shapeSdfCache = &threadContext->shapeSdfCache;

    for (int i = 0; i < 8; ++i) {
        if(shapeSDF[i] == INFINITY) {
            glm::vec3 p = min + length * Octree::getShift(i);
            shapeSDF[i] = evaluateSDF(args, shapeSdfCache, p);
        }
        if(resultSDF[i] == INFINITY) {
            float r = args.operation(existingResultSDF[i], shapeSDF[i]);
            resultSDF[i] = r;
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
            bool emptyNode = oldRoot->isEmpty() && !oldRoot->isSolid();
            bool emptyBlock = (oldBlock == NULL || oldBlock->isEmpty());

            if (emptyNode && emptyBlock) {
                if (oldBlock != NULL) {
                    oldBlock = oldRoot->deallocate(*allocator,oldBlock);
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
        const TexturePainter &painter,
        float minSize, 
        Simplifier &simplifier, 
        OctreeChangeHandler * changeHandler
    ) {
    threadsCreated = 0;
    *shapeCounter = 0;
    ShapeArgs args = ShapeArgs(SDF::opUnion, function, painter, model, simplifier, changeHandler, minSize);	
  	expand(args);
    OctreeNodeFrame frame = OctreeNodeFrame(root, *this, 0, root->sdf, DISCARD_BRUSH_INDEX, false);
    ThreadContext localChunkContext = ThreadContext(*this);
    shape(frame, args, &localChunkContext);
    std::cout << "\t\tOctree::add Ok! threads=" << threadsCreated << ", works=" << *shapeCounter << std::endl; 
}

void Octree::del(
        WrappedSignedDistanceFunction * function, 
        const Transformation model, 
        const TexturePainter &painter,
        float minSize, Simplifier &simplifier, OctreeChangeHandler  * changeHandler
    ) {
    threadsCreated = 0;
    *shapeCounter = 0;
    ShapeArgs args = ShapeArgs(SDF::opSubtraction, function, painter, model, simplifier, changeHandler, minSize);
    OctreeNodeFrame frame = OctreeNodeFrame(root, *this, 0, root->sdf, DISCARD_BRUSH_INDEX, false);
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

    if(check == ContainmentType::Disjoint) {
        SpaceType resultType = node ? node->getType() : SDF::eval(frame.sdf);
        return NodeOperationResult(node, SpaceType::Empty, resultType, NULL, NULL, false, false, DISCARD_BRUSH_INDEX);  // Skip this node
    }

    ChildBlock * block = NULL;
    float length = frame.cube.getLengthX();
    bool isChunk = isChunkNode(length);
    bool isLeaf = length <= args.minSize;
    if(node != NULL && !node->isLeaf()) {
        isLeaf = false;
    }

    NodeOperationResult children[8];
    std::vector<std::thread> threads;
    threads.reserve(8);
    if (!isLeaf) {
        bool isChildThread = isThreadNode(length*0.5f, args.minSize, 16);
        block = node ? node->getBlock(*allocator) : NULL;

        // --------------------------------
        // Iterate nodes and create threads
        // --------------------------------
        for (int i = 0; i < 8; ++i) {
            ChildBlock * block = node ? node->getBlock(*allocator) : NULL;
            OctreeNode * childNode = (node && block) ? block->get(i, *allocator) : NULL;
            if(node!=NULL && childNode == node) {
		        throw std::runtime_error("Infinite loop " + std::to_string((long)childNode) + " " + std::to_string((long)node));
            }
            
            float childSDF[8];
            bool interpolated = frame.interpolated;
            if(childNode) {
                SDF::copySDF(childNode->sdf, childSDF);
            } else {
                SDF::getChildSDF(frame.sdf, i, childSDF);
                interpolated |= true;
            }

            OctreeNodeFrame childFrame(
                childNode, 
                frame.cube.getChild(i), 
                frame.level + 1, 
                childSDF,
                node != NULL ? node->vertex.brushIndex : frame.brushIndex,
                interpolated
            );

            if(isChildThread) {
                ++threadsCreated;
                NodeOperationResult * result = &children[i];
                threads.emplace_back([this, childFrame, args, result]() {
                   ThreadContext localThreadContext(childFrame.cube);
                   *result = shape(childFrame, args, &localThreadContext);
                });
            } else {
                children[i] = shape(childFrame, args, threadContext);
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
    bool isSimplified = isLeaf;
    int brushIndex = frame.brushIndex;

    if(!isLeaf) {
        for(int i = 0; i < 8; ++i) {
            NodeOperationResult & child = children[i];
            if(child.process) {
                shapeSDF[i] = child.shapeSDF[i];
                resultSDF[i] = child.resultSDF[i];
            }
            childResultEmpty &= child.resultType == SpaceType::Empty;
            childResultSolid &= child.resultType == SpaceType::Solid;
            childShapeEmpty &= child.shapeType == SpaceType::Empty;
            childShapeSolid &= child.shapeType == SpaceType::Solid;
        }
    }

    // ------------------------------
    // Build SDFs based on inheritance/execution
    // ------------------------------

    buildSDF(args, frame.cube, shapeSDF, resultSDF, frame.sdf, threadContext);
    
    SpaceType shapeType = isLeaf ? SDF::eval(shapeSDF) : childToParent(childShapeSolid, childShapeEmpty);
    SpaceType resultType = isLeaf ? SDF::eval(resultSDF) : childToParent(childResultSolid, childResultEmpty);


    if(false && resultType != SpaceType::Surface) {
        // ------------------------------
        // Delete nodes if result is Empty
        // ------------------------------
        block = node ? node->clear(*allocator, args.changeHandler, block) : NULL;
        if(resultType == SpaceType::Empty) {
            node = node ? allocator->deallocate(node) : NULL;
            return NodeOperationResult(node, shapeType, resultType, resultSDF, shapeSDF, true, false, DISCARD_BRUSH_INDEX);
        }     
    }
    
    bool createdChildSurface = false;
    if(!isLeaf) {
        // ------------------------------
        // Simplification & Painting
        // ------------------------------
        if(!isSimplified) {
            isSimplified = args.simplifier.simplify(threadContext->cube, frame.cube, resultSDF, children);
        }
        if(!isSimplified) {
            for(int i =0 ; i < 8 ; ++i) {
                NodeOperationResult & child = children[i];
                OctreeNode * childNode = child.node;

                if(child.process) {
                    if(resultType == SpaceType::Surface && childNode == NULL && child.resultType != SpaceType::Empty) {
                        bool isChildLeaf = length*0.5f <= args.minSize;
                        BoundingCube childCube = frame.cube.getChild(i);
                        childNode = allocator->allocate()->init(Vertex(childCube.getCenter()));
                        childNode->setSolid(child.resultType == SpaceType::Solid);
                        childNode->setEmpty(child.resultType == SpaceType::Empty);
                        childNode->setSDF(child.resultSDF);
                        childNode->setLeaf(isChildLeaf);
                        childNode->setSimplified(false);
                        childNode->setChunk(false);
                        childNode->vertex.brushIndex = args.painter.paint(childNode->vertex);
                        createdChildSurface = true;
                        children[i].node = childNode;
                    }
                }
                if(frame.node != NULL && childNode == node) {
                    throw std::runtime_error("Infinite recursion! " + std::to_string((long) childNode) + " " + std::to_string((long)node) );
                }
            }
        }
    }

    if(shapeType != SpaceType::Empty || (frame.interpolated && node == NULL)) {   
        // ------------------------------
        // Created nodes if the shape is not Empty
        // ------------------------------     
        if(resultType == SpaceType::Surface && node == NULL) {
            node = allocator->allocate()->init(Vertex(frame.cube.getCenter()));   
        }        
        if(node!=NULL) {
            node->setSDF(resultSDF);
            node->setSolid(resultType == SpaceType::Solid);
            node->setEmpty(resultType == SpaceType::Empty);
            node->setChunk(isChunk);
            node->setSimplified(isSimplified);
            node->setLeaf(isLeaf);
            
            if(isChunk) {
                node->setDirty(true);
            }
            if(resultType == SpaceType::Surface) {
                node->vertex.position = glm::vec4(SDF::getAveragePosition(node->sdf, frame.cube) ,0.0f);
                node->vertex.normal = glm::vec4(SDF::getNormalFromPosition(node->sdf, frame.cube, node->vertex.position), 0.0f);
            }

            if(shapeType != SpaceType::Empty) {
                brushIndex = args.painter.paint(node->vertex);
            } 
            if(brushIndex == DISCARD_BRUSH_INDEX) {
                brushIndex = 0;
            }
            node->vertex.brushIndex = brushIndex;

            if(!isLeaf) {
                // ------------------------------
                // Created at least one solid child
                // ------------------------------
                if(block == NULL) {
                    block = node->allocate(*allocator)->init();
                }
                for(int i =0 ; i < 8 ; ++i) {
                    NodeOperationResult & child = children[i];
                    OctreeNode * childNode = child.node;
                    if(child.process) {
                        block->set(i, childNode, *allocator);
                    }
                }
            }

            // ------------------------------
            // Erase / Update handlers
            // ------------------------------           
            if(resultType != SpaceType::Surface) {
                if(isChunk && args.changeHandler != NULL) {
                    args.changeHandler->erase(node);
                }
            } else {
                if(isChunk && args.changeHandler != NULL) {
                    args.changeHandler->update(node);
                }
            }
            

        }
    }
    return NodeOperationResult(node, shapeType, resultType, resultSDF, shapeSDF, true, isSimplified, brushIndex);
}


void Octree::iterate(IteratorHandler &handler) {
    OctreeNodeData data(0, root, *this, NULL, root->sdf);
	handler.iterate(this, &data);
}

void Octree::iterateFlat(IteratorHandler &handler) {
    OctreeNodeData data(0, root, *this, NULL, root->sdf);
    handler.iterateFlatIn(this, &data);
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


#include "space.hpp"
#include "../math/SDF.hpp"

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

Octree::Octree(BoundingCube minCube, float chunkSize) : BoundingCube(minCube) {
    this->chunkSize = chunkSize;
	this->root = allocator.allocateOctreeNode(minCube)->init(glm::vec3(minCube.getCenter()));
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
    OctreeNode* node = root;
    BoundingCube cube = *this;
	if(!contains(pos)) {
		return NULL;
	}

    for (; node && level > 0; --level) {
        if (simplification && node->isSimplified()) {
            break;
        }
        int i = getNodeIndex(pos, cube);
        cube = cube.getChild(i);
        ChildBlock * block = node->getBlock(&allocator);
        node = node->getChildNode(i, &allocator, block);
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
    while (candidate) {
        node = candidate;
        if (simplification && node->isSimplified()) {
            break;
        }
        int i = getNodeIndex(pos, cube);
        cube = cube.getChild(i);
        ChildBlock * block = node->getBlock(&allocator);
        candidate = node->getChildNode(i, &allocator, block);
    }
    return node;
}

float Octree::getSdfAt(const glm::vec3 &pos) {
    OctreeNode * candidate = root;
    OctreeNode * node = candidate;
    BoundingCube candidateCube = *this;
    BoundingCube nodeCube = candidateCube;
    ChunkContext * chunk = NULL;


	if(!contains(pos)) {
		return INFINITY;
	}
    while (candidate) {
        node = candidate;
        nodeCube = candidateCube;
        if(chunk == NULL && nodeCube.getLengthX() <= chunkSize){
            glm::vec3 chunkKey = nodeCube.getMin();
            chunk = &chunks[chunkKey];
        }
        int i = getNodeIndex(pos, candidateCube);
        candidateCube = nodeCube.getChild(i);
        ChildBlock * block = node->getBlock(&allocator);
        candidate = node->getChildNode(i, &allocator, block);
    }

    if(node) {
        float sdf[8];
        getSdf(nodeCube, chunk, sdf);
        return SDF::interpolate(sdf, pos, nodeCube);
    }
    std::cerr << "Not interpolated" << std::endl;
    return INFINITY;
}

int Octree::getLevelAt(const glm::vec3 &pos, bool simplification) {
    OctreeNode* node = root;
    BoundingCube cube = *this;
	if(!contains(pos)) {
		return 0;
	}
    int level = 0;
    while (node) {
        if (simplification && node->isSimplified()) {
            break;
        }
        int i = getNodeIndex(pos, cube);
        cube = cube.getChild(i);
        ChildBlock * block = node->getBlock(&allocator);
        node = node->getChildNode(i, &allocator, block);
        ++level;
    }
    return level;
}

int Octree::getMaxLevel(BoundingCube &cube) {
    return getMaxLevel(this->root, cube, *this, 0);
}

int Octree::getMaxLevel(OctreeNode *node, BoundingCube &cube, BoundingCube &nodeCube, int level) {
    int l = level;
    if(!node->isSimplified()) {
        ChildBlock * block = node->getBlock(&allocator);

        for(int i=0; i < 8; ++i) {
            OctreeNode * n = node->getChildNode(i, &allocator, block);
            if(n!=NULL) {
                BoundingCube childCube = nodeCube.getChild(i); 
                if(nodeCube.intersects(childCube) && !cube.contains(childCube)) {
                    l = glm::max(l, getMaxLevel(n, cube, childCube, level + 1));
                }
            }
        }
    }
    return l;
}

int Octree::getNeighborLevel(OctreeNodeData &data, bool simplification, int direction) {
	int level = 0;
    for(int i=0; i < 8; ++i) {
		glm::vec3 pos = data.cube.getCenter() + direction* data.cube.getLengthX() * Octree::getShift(i);
        level = glm::max(level, getLevelAt(pos, simplification));
	}
    return level;
}

OctreeNode * Octree::fetch(OctreeNodeData &data, OctreeNode ** out, int i, bool simplification) {
    if(out[i] == NULL) {
        glm::vec3 pos = data.cube.getCenter() + data.cube.getLengthX() * Octree::getShift(i);
        out[i] = getNodeAt(pos, data.level, simplification);
    }
    return out[i];
}

template <typename T, std::size_t N> bool allDifferent(const T (&arr)[N]) {
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

void Octree::handleQuadNodes(OctreeNodeData &data, OctreeNodeTriangleHandler * handler, bool simplification) {

    OctreeNode * neighbors[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	
    for(size_t k =0 ; k < TESSELATION_EDGES.size(); ++k) {
		glm::ivec2 edge = TESSELATION_EDGES[k];
        uint mask = data.node->sign;
		bool sign0 = (mask & (0x1 << edge[0])) != 0x0;
		bool sign1 = (mask & (0x1 << edge[1])) != 0x0;

		if(sign0 != sign1) {
			glm::ivec4 quad = TESSELATION_ORDERS[k];
            Vertex vertices[4] = { Vertex(), Vertex(), Vertex(), Vertex() };
			for(int i =0; i<4 ; ++i) {
				OctreeNode * n = fetch(data, neighbors, quad[i], simplification);
                if(n != NULL && !n->isSolid() && !n->isEmpty()) {
                    vertices[i] = n->vertex;
                }else {
                    vertices[i].brushIndex = DISCARD_BRUSH_INDEX;
                }
			}
	
            if(allDifferent(vertices[0], vertices[2], vertices[1])) {
			    handler->handle(vertices[0], vertices[2], vertices[1], sign1);
			}
            if(allDifferent(vertices[0], vertices[3], vertices[2])) {
                handler->handle(vertices[0], vertices[3], vertices[2], sign1);
            }
		}
	}
}

glm::vec3 sdf_rotated(glm::vec3 p, glm::quat q, glm::vec3 pivot) {
    // Step 1: Translate point to rotation origin
    glm::vec3 local = p - pivot;

    // Step 2: Apply inverse rotation
    glm::vec3 rotated = glm::inverse(q) * local;

    // Step 3: Translate back
    return rotated + pivot;
}

void Octree::buildSDF(ShapeArgs * args, BoundingCube &cube, float * shapeSDF, float * resultSDF, ChunkContext * shapeChunkContext, ChunkContext * chunkContext) {
    const glm::vec3 min = cube.getMin();
    const glm::vec3 length = cube.getLength();
    std::unordered_map<glm::vec3, float> * shapeSdfCache = &shapeChunkContext->sdfCache;
    std::unordered_map<glm::vec3, float> * resultSdfCache = &chunkContext->sdfCache;

    for (int i = 0; i < 8; ++i) {
        glm::vec3 p = min + length * Octree::getShift(i);
        if(shapeSdfCache->find(p) != shapeSdfCache->end()) {
            resultSDF[i] = (*resultSdfCache)[p];
            shapeSDF[i] = (*shapeSdfCache)[p];
        } else {
            float existing = resultSdfCache->find(p) != resultSdfCache->end() ? (*resultSdfCache)[p] : INFINITY;
            float shape = args->function->distance(p, args->model);
            float result = args->operation(existing, shape);

            resultSDF[i] = result;
            shapeSDF[i] = shape;

            (*resultSdfCache)[p] = result;
            (*shapeSdfCache)[p] = shape;
        } 
    }
}

void Octree::expand(const WrappedSignedDistanceFunction *function, Transformation model) {
	while (!function->isContained(*this)) {
		glm::vec3 point = function->getCenter(model);
	    unsigned int i = getNodeIndex(point, *this) ^ 0x7;

	    setMin(getMin() -  Octree::getShift(i) * getLengthX());
	    setLength(getLengthX()*2);

	    if(root != NULL && root->isLeaf()) {
	    	allocator.deallocateOctreeNode(root, *this);
			root = NULL;
	    }
	    OctreeNode * newNode = allocator.allocateOctreeNode(*this)->init(getCenter());
        ChildBlock * block = newNode->createBlock(&allocator);
		newNode->setChildNode(i, root, &allocator, block);
	    root = newNode;
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
	expand(function, model);	
    *counter = 0;
    *threadId = 0;
    *works = 0;
    OctreeNodeFrame frame = OctreeNodeFrame(root, *this, minSize, 0);
    ShapeArgs args = ShapeArgs(SDF::opUnion, function, painter, model, simplifier, changeHandler);
    ShapeContext context = ShapeContext(0);
    shape(context, frame, &args, NULL, NULL);
    std::cout << "\t\tOctree::add Ok! threads=" << *threadId << ", works=" << *works << std::endl; 
}

void Octree::del(
        WrappedSignedDistanceFunction * function, 
        const Transformation model, 
        const TexturePainter &painter,
        float minSize, Simplifier &simplifier, OctreeChangeHandler  * changeHandler
    ) {
    *counter = 0;
    *threadId = 0;
    *works = 0;
    OctreeNodeFrame frame = OctreeNodeFrame(root, *this, minSize, 0);
    ShapeArgs args = ShapeArgs(SDF::opSubtraction, function, painter, model, simplifier, changeHandler);
    ShapeContext context = ShapeContext(0);
    shape(context, frame, &args, NULL, NULL);
    std::cout << "\t\tOctree::del Ok! threads=" << *threadId << ", works=" << *works << std::endl; 
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

void Octree::getSdf(BoundingCube cube, ChunkContext * chunk, float * resultSDF) {
    for(int i = 0 ; i < 8; ++i) {
        glm::vec3 p = cube.getMin() + cube.getLength() * Octree::getShift(i);
        resultSDF[i] = chunk != NULL && chunk->sdfCache.find(p) != chunk->sdfCache.end() ? chunk->sdfCache[p] : INFINITY;
    }
}


NodeOperationResult Octree::shape(ShapeContext context, OctreeNodeFrame frame, ShapeArgs * args, ChunkContext * shapeChunkContext, ChunkContext * chunkContext) {    
    ContainmentType check = args->function->check(frame.cube);
    OctreeNode * node  = frame.node;
    ChunkContext localChunkContext(frame.cube);
    bool childResultSolid = true;
    bool childResultEmpty = true;
    bool childShapeSolid = true;
    bool childShapeEmpty = true;
    bool chunkNode = false;
    ChildBlock * block = NULL;

    if(check == ContainmentType::Disjoint) {
        SpaceType spaceType = node ? node->getType() : SpaceType::Empty;
        return NodeOperationResult(node, SpaceType::Empty, spaceType, NULL, false);  // Skip this node
    }

    if(frame.cube.getLengthX() <= chunkSize){
        if(chunkContext == NULL){
            glm::vec3 chunkKey = frame.cube.getMin();
            if(chunks.find(chunkKey) == chunks.end()) {
                chunks[chunkKey] = ChunkContext(frame.cube);
            }
            chunkContext = &chunks[chunkKey];
        }

        if(shapeChunkContext == NULL) {
            shapeChunkContext = &localChunkContext;
            chunkNode = true;
        }
    }

    bool isLeaf = frame.cube.getLengthX() <= frame.minSize;
    NodeOperationResult children[8];
    std::vector<std::thread> threads;
    threads.reserve(MAX_CONCURRENCY);
    if (!isLeaf) {
        block = node ? node->getBlock(&allocator) : NULL;
        for (int i = 7; i >= 0; --i) {
            OctreeNode * childNode = node ? node->getChildNode(i, &allocator, block) : NULL;
            int currentCounter = (*counter);
            float len = frame.cube.getLengthX();
            BoundingCube childCube = frame.cube.getChild(i);

            OctreeNodeFrame childFrame(
                childNode, 
                childCube, 
                frame.minSize, 
                frame.level + 1 
            );

            bool isChildChunk = len*0.5f <= chunkSize && len > chunkSize;
            (*works)++;

            if(isChildChunk && (currentCounter=(*counter)++) < MAX_CONCURRENCY) {
                context.threadId = (*threadId)++;
                threads.emplace_back([this, i, context, childFrame, args, &children, shapeChunkContext, chunkContext]() {
                   NodeOperationResult * result = children+i;
                   *result = shape(context, childFrame, args, shapeChunkContext, chunkContext);
                });
            } else {
                children[i] = shape(context, childFrame, args, shapeChunkContext, chunkContext);
            }
        }
    }
    int joinCount = 0;
    for(std::thread &t : threads) {
        if(t.joinable()) {
            t.join();
            ++joinCount;
        }
    }
    if(joinCount > 0) {
        *counter -= joinCount;
    }

    // build SDF from children
    for(int i = 0; i < 8; ++i) {
        NodeOperationResult & child = children[i];

        childResultEmpty &= child.resultType == SpaceType::Empty;
        childResultSolid &= child.resultType == SpaceType::Solid;
        childShapeEmpty &= child.shapeType == SpaceType::Empty;
        childShapeSolid &= child.shapeType == SpaceType::Solid;
    }

    // Process Shape
    
    float resultSDF[8];
    SpaceType shapeType;
    SpaceType resultType;

    if(isLeaf && chunkContext != NULL) {
        float shapeSDF[8];
        buildSDF(args, frame.cube, shapeSDF, resultSDF, shapeChunkContext, chunkContext);

        shapeType = SDF::eval(shapeSDF);
        resultType = SDF::eval(resultSDF);
    } else {
        getSdf(frame.cube, chunkContext, resultSDF);
        shapeType = childToParent(childShapeSolid, childShapeEmpty);
        resultType = childToParent(childResultSolid, childResultEmpty);
    }

    // Take action
    if(resultType == SpaceType::Surface && node == NULL) {
        node = allocator.allocateOctreeNode(frame.cube)->init(Vertex(frame.cube.getCenter()));   
    }
    if(node!=NULL) {
        node->setSdf(resultSDF);
        //TODO also write to SDF storage
        node->setSolid(resultType == SpaceType::Solid);
        node->setEmpty(resultType == SpaceType::Empty);
        node->setChunk(chunkNode);
        node->setLeaf(isLeaf);
        if(resultType == SpaceType::Surface) {
            node->vertex.position = glm::vec4(SDF::getAveragePosition(resultSDF, frame.cube) ,0.0f);
            node->vertex.normal = glm::vec4(SDF::getNormalFromPosition(resultSDF, frame.cube, node->vertex.position), 0.0f);
        }
        if(!isLeaf) {
            if(block == NULL) {
                block = node->createBlock(&allocator);
            }
            for(int i =0 ; i < 8 ; ++i) {
                NodeOperationResult & child = children[i];
                if(child.process) {
                    OctreeNode * childNode = child.node;
                    if(resultType == SpaceType::Surface && childNode == NULL && child.resultType != SpaceType::Surface) {
                        BoundingCube childCube = frame.cube.getChild(i);
                        childNode = allocator.allocateOctreeNode(childCube)->init(Vertex(childCube.getCenter()));
                        childNode->setSolid(child.resultType == SpaceType::Solid);
                        childNode->setEmpty(child.resultType == SpaceType::Empty);
                        childNode->setSdf(child.sdf);
                        childNode->setLeaf(false);
                        childNode->setChunk(false);
                    }
                    node->setChildNode(i, childNode, &allocator, block);
                }
            }
        }

        if(resultType != SpaceType::Surface) {
            if(chunkNode) {
                if(args->changeHandler != NULL) {
                    args->changeHandler->erase(node);
                }
            }
            node->clear(&allocator, frame.cube, args->changeHandler);
        } else {
            if(chunkNode) {
                if(args->changeHandler != NULL) {
                    args->changeHandler->update(node);
                }
                node->setDirty(true);
            }
        }

        if(shapeChunkContext != NULL) {
            args->simplifier.simplify(this, shapeChunkContext->cube, OctreeNodeData(frame.level, node, frame.cube, NULL, chunkContext));
            if(node->isSimplified()) {
                if(shapeType != SpaceType::Empty) {
                    args->painter.paint(node->vertex);
                }
            }
        }
    }

    return NodeOperationResult(node, shapeType, resultType, resultSDF, true);
}

void Octree::iterate(IteratorHandler &handler) {
	BoundingCube cube(glm::vec3(getMinX(),getMinY(),getMinZ()),getLengthX());
	handler.iterate(this, OctreeNodeData(0, root, cube, NULL, NULL));
}

void Octree::iterateFlat(IteratorHandler &handler) {
	BoundingCube cube(glm::vec3(getMinX(),getMinY(),getMinZ()),getLengthX());
    handler.iterateFlatIn(this, OctreeNodeData(0,root, cube, NULL, NULL));
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
    root->exportSerialization(&allocator, nodes, &leafNodes, *this, *this, 0u);
	std::cout << "exportNodesSerialization Ok!" << std::endl;
}


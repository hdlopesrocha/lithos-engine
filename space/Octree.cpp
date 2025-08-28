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

	if(!contains(pos)) {
		return INFINITY;
	}
    while (candidate) {
        node = candidate;
        nodeCube = candidateCube;
        int i = getNodeIndex(pos, candidateCube);
        candidateCube = nodeCube.getChild(i);
        ChildBlock * block = node->getBlock(&allocator);
        candidate = node->getChildNode(i, &allocator, block);
    }

    if(node) {
        return SDF::interpolate(node->sdf, pos, nodeCube);
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
        uint mask = data.node->mask;
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

void Octree::buildSDF(SignedDistanceFunction * function, Transformation model, BoundingCube &cube, float * resultSDF, float * existingSDF) {
    const glm::vec3 min = cube.getMin();
    const glm::vec3 length = cube.getLength();
    for (int i = 0; i < 8; ++i) {
        if(existingSDF != NULL && existingSDF[i] != INFINITY) {
            resultSDF[i] = existingSDF[i];
        } else {
            glm::vec3 p = min + length * Octree::getShift(i);
            resultSDF[i] = function->distance(p, model);
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
    ShapeArgs args = ShapeArgs(SDF::opUnion, function, painter, model, OctreeNodeFrame(root, *this, minSize, 0, root->sdf, SpaceType::Surface), NULL, simplifier, changeHandler);
    ShapeContext context = ShapeContext(0);
    shape(context, args);
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
    ShapeArgs args = ShapeArgs(SDF::opSubtraction, function, painter, model, OctreeNodeFrame(root, *this, minSize, 0, root->sdf, SpaceType::Surface), NULL, simplifier, changeHandler);
    ShapeContext context = ShapeContext(0);
    shape(context, args);
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


void Octree::shapeChild(ShapeContext context, ShapeArgs args, ShapeChildArgs childArgs) {
    int w = (*works)++;
    /*if(w % 901143 == 0) {
        std::cout << "\twork=" << w << ", thread=" << context.threadId 
                    << ", min={" << args.frame.cube.getMinX() << ", " << args.frame.cube.getMinY() << ", " << args.frame.cube.getMinZ() << "}"
                    << ", len={" << args.frame.cube.getLengthX() << ", " << args.frame.cube.getLengthY() << ", " << args.frame.cube.getLengthZ() << "}"
                    << std::endl;
    }*/
    
    OctreeNode * node  = args.frame.node;
    OctreeNode * childNode = node ? node->getChildNode(childArgs.i, &allocator, childArgs.block) : NULL;
    
    float childSDF[8];
    SpaceType childType;
    if(childNode) {
        SDF::copySDF(childNode->sdf, childSDF);
        if(childNode->isSolid()) {
            childType = SpaceType::Solid;
        } else if(childNode->isEmpty()) {
            childType = SpaceType::Empty;
        } else {
            childType = SpaceType::Surface;
        }
    } else {
        SDF::getChildSDF(args.frame.sdf, childArgs.i, childSDF);
        childType = SDF::eval(childSDF);
    }

    ShapeArgs newChildArgs = ShapeArgs(
        args.operation, 
        args.function, 
        args.painter, 
        args.model, 
        OctreeNodeFrame(
            childNode, 
            args.frame.cube.getChild(childArgs.i), 
            args.frame.minSize, 
            args.frame.level + 1, 
            childSDF, 
            childType), 
        args.chunk, 
        args.simplifier, 
        args.changeHandler);

    NodeOperationResult child = shape(context, newChildArgs);

    childArgs.parentShapeSDF[childArgs.i] = childNode != NULL? child.shapeSDF[childArgs.i] : INFINITY;
    childArgs.children[childArgs.i] = child;
    childArgs.childResultEmpty.fetch_and(child.resultType == SpaceType::Empty);
    childArgs.childResultSolid.fetch_and(child.resultType == SpaceType::Solid);
    childArgs.childShapeEmpty.fetch_and(child.shapeType == SpaceType::Empty);
    childArgs.childShapeSolid.fetch_and(child.shapeType == SpaceType::Solid);
    childArgs.childProcess.fetch_or(child.process);
}

NodeOperationResult Octree::shape(ShapeContext context, ShapeArgs args) {    
    ContainmentType check = args.function->check(args.frame.cube);
    OctreeNode * node  = args.frame.node;
    bool chunkNode = false;
    if(check == ContainmentType::Disjoint) {
        return NodeOperationResult(args.frame.cube, node, SpaceType::Empty, args.frame.type, args.frame.sdf, args.frame.sdf, false, false);  // Skip this node
    }
    if(args.chunk == NULL && args.frame.cube.getLengthX() <= chunkSize){
        args.chunk = &args.frame.cube;
        chunkNode = true;
    }
    bool isLeaf = args.frame.cube.getLengthX() <= args.frame.minSize;
    NodeOperationResult children[8];
    std::atomic<int> childResultSolid(true);
    std::atomic<int> childResultEmpty(true);
    std::atomic<int> childShapeSolid(true);
    std::atomic<int> childShapeEmpty(true);
    std::atomic<int> childProcess(false);
    float parentShapeSDF[8] ={INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY};
    ChildBlock * block = NULL;
    std::vector<std::thread> threads;
    threads.reserve(MAX_CONCURRENCY);
    if (!isLeaf) {

        block = node ? node->getBlock(&allocator) : NULL;
        for (int i = 7; i >= 0; --i) {
        
            ShapeChildArgs childArgs = ShapeChildArgs(children, childResultSolid, childResultEmpty, childShapeSolid, childShapeEmpty, childProcess, parentShapeSDF, block, i);
            int currentCounter = (*counter);
            float len = args.frame.cube.getLengthX();

            bool isChildChunk = len*0.5f <= chunkSize && len > chunkSize;
            if(isChildChunk && (currentCounter=(*counter)++) < MAX_CONCURRENCY){
                context.threadId = (*threadId)++;
                /*std::cout << "+ Create thread(" << context.threadId << "," << args.frame.level << ")" 
                            << ", index=" << i
                            << ", min={" << args.frame.cube.getMinX() << ", " << args.frame.cube.getMinY() << ", " << args.frame.cube.getMinZ() << "}"
                            << ", len={" << args.frame.cube.getLengthX() << ", " << args.frame.cube.getLengthY() << ", " << args.frame.cube.getLengthZ() << "}"
                            << std::endl;*/
                threads.emplace_back([this, context, args, childArgs]() {
                    shapeChild(context, args, childArgs);
                    //semaphore.release();
                    //std::cout << "- End thread(" << context.threadId << "," << args.frame.level << ")" << std::endl;

                });
            }else {
                //std::cout << "Call method(" << currentCounter << ")" << std::endl;
                shapeChild(context, args, childArgs);
            }
      
        }
    }
    int joinCount = 0;
    for(std::thread &t : threads){
        if(t.joinable()){
            t.join();
            ++joinCount;
        }
    }
    *counter -= joinCount;
    // Process Shape
    float shapeSDF[8];
    buildSDF(args.function, args.model, args.frame.cube, shapeSDF, parentShapeSDF);

    SpaceType shapeType = isLeaf ? SDF::eval(shapeSDF) : childToParent(childShapeSolid, childShapeEmpty);
    // Process Result
    float resultSDF[8];
    for(int i = 0; i < 8; ++i) {
        resultSDF[i] = args.operation(args.frame.sdf[i], shapeSDF[i]);
    }
    SpaceType resultType = isLeaf ? SDF::eval(resultSDF) : childToParent(childResultSolid, childResultEmpty);
        
    // Take action
    if(resultType == SpaceType::Surface && node == NULL) {
        node = allocator.allocateOctreeNode(args.frame.cube)->init(Vertex(args.frame.cube.getCenter()));   
    }
    if(node!=NULL) {
        node->setSdf(resultSDF);
        node->setSolid(resultType == SpaceType::Solid);
        node->setEmpty(resultType == SpaceType::Empty);
        node->setChunk(chunkNode);
        node->setLeaf(isLeaf);
        if(resultType == SpaceType::Surface) {
            node->vertex.position = glm::vec4(SDF::getAveragePosition(node->sdf, args.frame.cube) ,0.0f);
            //node->vertex.normal = glm::vec4(SDF::getNormal(node->sdf, args.frame.cube), 0.0f);   
            node->vertex.normal = glm::vec4(SDF::getNormalFromPosition(node->sdf, args.frame.cube, node->vertex.position), 0.0f);
        }
        if(!isLeaf) {
            if(block == NULL) {
                block = node->createBlock(&allocator);
            }
            if(childProcess) {
                for(int i =0 ; i < 8 ; ++i) {
                    NodeOperationResult child = children[i];
                    OctreeNode * childNode = child.node;
                    if(resultType==SpaceType::Surface && childNode == NULL && child.resultType != SpaceType::Surface) {
                        childNode = allocator.allocateOctreeNode(child.cube)->init(Vertex(child.cube.getCenter()));
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
            if(chunkNode || args.chunk == NULL) {
                if(args.changeHandler != NULL) {
                    args.changeHandler->erase(node);
                }
            }
            node->clear(&allocator, args.frame.cube, args.changeHandler);
        } else {
            if(chunkNode || args.chunk == NULL) {
                if(args.changeHandler != NULL) {
                    args.changeHandler->update(node);
                }
                node->setDirty(true);
            }
        }

        if(args.chunk != NULL) {
            args.simplifier.simplify(this, *args.chunk, OctreeNodeData(args.frame.level, node, args.frame.cube, NULL));
            if(node->isSimplified()) {
                if(shapeType != SpaceType::Empty) {
                    args.painter.paint(node->vertex);
                }
            }
        }
    }

    return NodeOperationResult(args.frame.cube, node, shapeType, resultType, resultSDF, shapeSDF, isLeaf, true);
}

void Octree::iterate(IteratorHandler &handler) {
	BoundingCube cube(glm::vec3(getMinX(),getMinY(),getMinZ()),getLengthX());
	handler.iterate(this, OctreeNodeData(0, root, cube, NULL));
}

void Octree::iterateFlat(IteratorHandler &handler) {
	BoundingCube cube(glm::vec3(getMinX(),getMinY(),getMinZ()),getLengthX());
    handler.iterateFlatIn(this, OctreeNodeData(0,root, cube, NULL));
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


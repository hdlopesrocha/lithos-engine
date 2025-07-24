#include "space.hpp"
#include "../math/SDF.hpp"

//      6-----7
//     /|    /|
//    4z+---5 |
//    | 2y--+-3
//    |/    |/
//    0-----1x

static std::vector<glm::ivec4> tessOrder;
static std::vector<glm::ivec2> tessEdge;
static bool initialized = false;


static void initialize() {
    if(!initialized) {
        tessOrder.push_back(glm::ivec4(0,1,3,2));tessEdge.push_back(glm::ivec2(3,7));
        tessOrder.push_back(glm::ivec4(0,2,6,4));tessEdge.push_back(glm::ivec2(6,7));
        tessOrder.push_back(glm::ivec4(0,4,5,1));tessEdge.push_back(glm::ivec2(5,7));
        initialized = true;
    }
}

Octree::Octree() : BoundingCube(glm::vec3(0.0f), 1.0f) {
    this->chunkSize = 1.0f;
    this->root = NULL;
    initialize();
}

Octree::Octree(BoundingCube minCube, float chunkSize) : BoundingCube(minCube){
    this->chunkSize = chunkSize;
	this->root = allocator.allocateOctreeNode(minCube)->init(glm::vec3(minCube.getCenter()));
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

OctreeNode * Octree::fetch(OctreeNodeData &data, OctreeNode ** out, int i) {
    int direction = 1;
    bool simplification = true;
    if(out[i] == NULL) {
        glm::vec3 pos = data.cube.getCenter() + direction * data.cube.getLengthX() * Octree::getShift(i);
        out[i] = getNodeAt(pos, data.level, simplification);
    }
    return out[i];
}


void Octree::handleQuadNodes(OctreeNodeData &data, OctreeNodeTriangleHandler * handler, bool simplification) {

    OctreeNode * neighbors[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	
    for(size_t k =0 ; k < tessOrder.size(); ++k) {
		glm::ivec2 edge = tessEdge[k];
        float * sdf = data.node->sdf;

        float d0 = sdf[edge[0]];
        float d1 = sdf[edge[1]];

		bool sign0 = d0 < 0.0f;
		bool sign1 = d1 < 0.0f;

		if(sign0 != sign1) {
			glm::ivec4 quad = tessOrder[k];
            Vertex vertices[4] = { Vertex(), Vertex(), Vertex(), Vertex() };
			for(int i =0; i<4 ; ++i) {
				OctreeNode * n = fetch(data, neighbors, quad[i]);
                if(n != NULL && !n->isSolid() && !n->isEmpty()) {
                    vertices[i] = n->vertex;
                }else {
                    vertices[i].brushIndex = DISCARD_BRUSH_INDEX;
                }
			}
	
			handler->handle(vertices[0], vertices[2], vertices[1], sign1);
			handler->handle(vertices[0], vertices[3], vertices[2], sign1);
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

void Octree::buildSDF(SignedDistanceFunction &function, Transformation model, BoundingCube &cube, float * resultSDF, float * existingSDF) {
    const glm::vec3 min = cube.getMin();
    const glm::vec3 length = cube.getLength();
    for (int i = 0; i < 8; ++i) {
        if(existingSDF != NULL && existingSDF[i] != INFINITY) {
            resultSDF[i] = existingSDF[i];
        } else {
            glm::vec3 p = min + length * Octree::getShift(i);
            resultSDF[i] = function.distance(p, model);
        }
    }
}

void Octree::expand(const WrappedSignedDistanceFunction &function, Transformation model) {
	while (!function.isContained(*this)) {
		glm::vec3 point = function.getCenter(model);
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
    WrappedSignedDistanceFunction &function, 
    const Transformation model, 
    const TexturePainter &painter,
    float minSize, Simplifier &simplifier, OctreeChangeHandler * changeHandler) {
	expand(function, model);	
    shape(SDF::opUnion, function, painter, model, OctreeNodeFrame(root, *this, minSize, 0, root->sdf, SpaceType::Surface ), NULL, simplifier, changeHandler);
}

void Octree::del(
    WrappedSignedDistanceFunction &function, 
    const Transformation model, 
    const TexturePainter &painter,
    float minSize, Simplifier &simplifier, OctreeChangeHandler  * changeHandler) {
    shape(SDF::opSubtraction, function, painter, model, OctreeNodeFrame(root, *this, minSize, 0, root->sdf, SpaceType::Surface), NULL, simplifier, changeHandler);
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

NodeOperationResult Octree::shape(
    float (*operation)(float, float),
    WrappedSignedDistanceFunction &function, 
    const TexturePainter &painter,
    const Transformation model,
    OctreeNodeFrame frame, BoundingCube * chunk, Simplifier &simplifier, OctreeChangeHandler * changeHandler) {
    ContainmentType check = function.check(frame.cube);
    OctreeNode * node  = frame.node;
    bool chunkNode = false;
    if(check == ContainmentType::Disjoint) {
        return NodeOperationResult(frame.cube, node, SpaceType::Empty, frame.type, frame.sdf, frame.sdf, false);  // Skip this node
    }
    if(chunk == NULL && frame.cube.getLengthX() < chunkSize){
        chunk = &frame.cube;
        chunkNode = true;
    }
    bool isLeaf = frame.cube.getLengthX() <= frame.minSize;
    NodeOperationResult children[8];
    bool childResultSolid = true;
    bool childResultEmpty = true;
    bool childShapeSolid = true;
    bool childShapeEmpty = true;
    bool childProcess = false;
    float parentShapeSDF[8] ={INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY};

    ChildBlock * block = NULL;
    if (!isLeaf) {
        block = node ? node->getBlock(&allocator) : NULL;
        for (int i = 7; i >= 0; --i) {
            OctreeNode * childNode = node ? node->getChildNode(i, &allocator, block) : NULL;
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
                SDF::getChildSDF(frame.sdf, i, childSDF);
                childType = SDF::eval(childSDF);
            }
            NodeOperationResult child = shape(operation, function, painter, 
                model, OctreeNodeFrame(childNode, frame.cube.getChild(i), frame.minSize, frame.level + 1, childSDF, childType), 
                chunk, simplifier, changeHandler);

            parentShapeSDF[i] = childNode != NULL? child.shapeSDF[i] : INFINITY;
            children[i] = child;
            childResultEmpty &= child.resultType == SpaceType::Empty;
            childResultSolid &= child.resultType == SpaceType::Solid;
            childShapeEmpty &= child.shapeType == SpaceType::Empty;
            childShapeSolid &= child.shapeType == SpaceType::Solid;
            childProcess |= child.process;
        }
    }

    // Process Shape
    float shapeSDF[8];
    buildSDF(function, model, frame.cube, shapeSDF, parentShapeSDF);


    SpaceType shapeType = isLeaf ? SDF::eval(shapeSDF) : childToParent(childShapeSolid, childShapeEmpty);

    // Process Result
    float resultSDF[8];
    for(int i = 0; i < 8; ++i) {
        resultSDF[i] = operation(frame.sdf[i], shapeSDF[i]);
    }
    SpaceType resultType = isLeaf ? SDF::eval(resultSDF) : childToParent(childResultSolid, childResultEmpty);
        
    // Take action
    if(resultType == SpaceType::Surface && node == NULL) {
        node = allocator.allocateOctreeNode(frame.cube)->init(Vertex(frame.cube.getCenter()));   
    }
    if(node!=NULL) {
        node->setSdf(resultSDF);
        node->setSolid(resultType == SpaceType::Solid);
        node->setEmpty(resultType == SpaceType::Empty);
        if(resultType == SpaceType::Surface) {
            //node->vertex.normal = SDF::getNormalFromPosition(node->sdf, frame.cube, node->vertex.position);
            node->vertex.position = glm::vec4(SDF::getPosition(node->sdf, frame.cube) ,0.0f);
            node->vertex.normal = glm::vec4(SDF::getNormal(node->sdf, frame.cube), 0.0f);       
        }
        if(shapeType != SpaceType::Empty) {
            painter.paint(node->vertex);
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
                    }
                    node->setChildNode(i, childNode, &allocator, block);
                }
            }
        }

        if(resultType != SpaceType::Surface) {
            if(chunkNode) {
                if(changeHandler != NULL) {
                    changeHandler->erase(node);
                }
            }
            node->clear(&allocator, frame.cube);
        } else {
            if(chunkNode) {
                if(changeHandler != NULL) {
                    changeHandler->update(node);
                }
                node->setDirty(true);
            }
        }

        if(chunk != NULL) {
            simplifier.simplify(*chunk, OctreeNodeData(frame.level, chunkSize, node, frame.cube, NULL, &allocator));  
        }
    }
    return NodeOperationResult(frame.cube, node, shapeType, resultType, resultSDF, shapeSDF, true);
}

void Octree::iterate(IteratorHandler &handler) {
	BoundingCube cube(glm::vec3(getMinX(),getMinY(),getMinZ()),getLengthX());
	handler.iterate(OctreeNodeData(0, chunkSize, root, cube, NULL, &this->allocator));
}

void Octree::iterateFlat(IteratorHandler &handler) {
	BoundingCube cube(glm::vec3(getMinX(),getMinY(),getMinZ()),getLengthX());
    handler.iterateFlatIn(OctreeNodeData(0, chunkSize,root, cube, NULL, &this->allocator));
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
    root->exportSerialization(&allocator, nodes, *this);
	std::cout << "exportNodesSerialization Ok!" << std::endl;
}


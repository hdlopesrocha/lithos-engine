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

Octree::Octree(BoundingCube minCube, float chunkSize) : BoundingCube(minCube){
    this->chunkSize = chunkSize;
	this->root = allocator.allocateOctreeNode(minCube)->init(glm::vec3(minCube.getCenter()));
	if(!initialized) {
		tessOrder.push_back(glm::ivec4(0,1,3,2));tessEdge.push_back(glm::ivec2(3,7));
		tessOrder.push_back(glm::ivec4(0,2,6,4));tessEdge.push_back(glm::ivec2(6,7));
		tessOrder.push_back(glm::ivec4(0,4,5,1));tessEdge.push_back(glm::ivec2(5,7));
		initialized = true;
	}
}

int getNodeIndex(const glm::vec3 &vec, const BoundingCube &cube) {
	glm::vec3 c = cube.getCenter();
    return (vec.x >= c.x ? 4 : 0) + (vec.y >= c.y ? 2 : 0) + (vec.z >= c.z ? 1 : 0);
}

ContainmentType Octree::contains(const AbstractBoundingBox &c) {
    OctreeNode* node = root;
    BoundingCube cube = *this;

    while (node) {
        bool allContains = true;
        bool anyIntersection = false;
        OctreeNode* candidate = NULL;
        BoundingCube candidateCube;
        ChildBlock * block = node->getBlock(&allocator);

        for(int i =0 ; i < 8 ; ++i) {
            OctreeNode* subNode = node->getChildNode(i, &allocator, block);
            if(subNode != NULL) {
                BoundingCube subCube = cube.getChild(i);
                bool cubeIntersects = subCube.intersects(c);
                if(cubeIntersects) {               
                    candidate = subNode;
                    candidateCube = subCube;
                    anyIntersection = true;
                    if(!candidate->isSolid()) {
                        allContains = false;
                    }
                }
            }
        }
        if (!anyIntersection) {
            return ContainmentType::Disjoint;
        }
        if (allContains) {
            return ContainmentType::Contains;
        }
        node = candidate;
        cube = candidateCube;
    }
    if (node == NULL){
		return ContainmentType::Disjoint;
	} 

    return node->isSolid() ? ContainmentType::Contains : ContainmentType::Intersects;
}

ContainmentType Octree::contains(const glm::vec3 &pos) {
    OctreeNode* node = root;
    BoundingCube cube = *this;

    while (node) {
        bool testResult = cube.contains(pos);

        // If completely outside, return Disjoint immediately
        if (!testResult) {
            return ContainmentType::Disjoint;
        }

        // If the node is marked as solid and the cube is not Disjoint, return Contains
        if (node->isSolid()) {
            break;
        }
		
		int i = getNodeIndex(pos, cube);
        if (i < 0) {
            return ContainmentType::Disjoint;
        }
        ChildBlock * block = node->getBlock(&allocator);
   		OctreeNode* candidate = node->getChildNode(i, &allocator, block);
        if (candidate == NULL) {
            break;
        }

        cube = cube.getChild(i);
        node = candidate;
    }
    if (node == NULL){
		return ContainmentType::Disjoint;
	} 

    return node->isSolid() ? ContainmentType::Contains : ContainmentType::Intersects;
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
		bool sign0 = (sdf[edge[0]]) < 0.0f;
		bool sign1 = (sdf[edge[1]]) < 0.0f;

		if(sign0 != sign1) {
			glm::ivec4 quad = tessOrder[k];
			OctreeNode * quads[4] = {NULL, NULL, NULL, NULL};
			for(int i =0; i<4 ; ++i) {
				OctreeNode * n = fetch(data, neighbors, quad[i]);
				quads[i] = (n != NULL && !n->isSolid()) ? n : NULL;
                if(n == NULL) {
                    break;
                }
			} 

			handler->handle(quads[0],quads[2],quads[1],sign1);
			handler->handle(quads[0],quads[3],quads[2],sign1);
		}
	}
}

void buildSDF(const ContainmentHandler &handler, BoundingCube &cube, float * resultSDF) {
    const glm::vec3 min = cube.getMin();
    const float lengthX = cube.getLengthX();
    for (int i = 0; i < 8; ++i) {
        resultSDF[i] = handler.distance(min + lengthX * Octree::getShift(i));
    }
}

bool isSdfSurface(float * sdf) {
    bool hasPositive = false;
    bool hasNegative = false;
    for (int i = 0; i < 8; ++i) {  
        if (sdf[i] >= 0.0f) {
            hasPositive = true;
        } else {
            hasNegative = true;
        }
    }
    return hasNegative && hasPositive;
}

bool isSdfSolid(float * sdf) {
    bool hasPositive = false;
    bool hasNegative = false;
    for (int i = 0; i < 8; ++i) {  
        if (sdf[i] >= 0.0f) {
            hasPositive = true;
        } else {
            hasNegative = true;
        }
    }
    return hasNegative && !hasPositive;
}

bool isSdfEmpty(float * sdf) {
    bool hasPositive = false;
    bool hasNegative = false;
    for (int i = 0; i < 8; ++i) {  
        if (sdf[i] >= 0.0f) {
            hasPositive = true;
        } else {
            hasNegative = true;
        }
    }
    return !hasNegative && hasPositive;
}

void Octree::expand(const ContainmentHandler &handler) {
	while (!handler.isContained(*this)) {
		Vertex vertex(getCenter());
	   
        float minSDF = handler.distance(getMin());
        unsigned int minIndex = 0;
        for(unsigned int i=1; i < 8 ; ++i) {
            glm::vec3 corner = getMin() + Octree::getShift(i) * getLengthX();
            float sdf = handler.distance(corner);
            if(sdf < minSDF) {
                minSDF = sdf;
                minIndex = i;
            }
        }
	    unsigned int i = minIndex ^ 0x7;

	    setMin(getMin() -  Octree::getShift(i) * getLengthX());
	    setLength(getLengthX()*2);

	    if(root != NULL && root->isLeaf()) {
	    	allocator.deallocateOctreeNode(root, *this);
			root = NULL;
	    }
	    OctreeNode * newNode = allocator.allocateOctreeNode(*this)->init(getCenter());
		newNode->setChildNode(i, root, &allocator);
	    root = newNode;
	}
}

bool isNodeOutside(const ContainmentHandler &handler, BoundingCube &cube) {
    float centerSDF = handler.distance(cube.getCenter());
    float l = cube.getLengthX();
    float r = l * SQRT_3_OVER_2;
    glm::vec3 c = cube.getCenter();
    return handler.distance(c) > r * 1.2f;
}

void deleteNode(OctreeNode * node, OctreeAllocator &allocator, BoundingCube &cube) {
    node->clear(&allocator, cube);
    allocator.deallocateOctreeNode(node, cube);
}

void Octree::add(const ContainmentHandler &handler, const OctreeNodeDirtyHandler &dirtyHandler, float minSize, Simplifier &simplifier) {
	expand(handler);	
    shape(handler, dirtyHandler, OctreeNodeFrame(root, -1, *this, minSize, 0, root->isSolid()), NULL, simplifier, true);
}

NodeOperationResult Octree::shape(const ContainmentHandler &handler, const OctreeNodeDirtyHandler &dirtyHandler, OctreeNodeFrame frame, BoundingCube * chunk, Simplifier &simplifier, bool isAdd) {
    OctreeNode * node  = frame.node;
    bool isLeaf = frame.cube.getLengthX() <= frame.minSize;
    bool isSolid = node ? node->isSolid() : frame.isSolid;
    float currentSDF[8];
    buildSDF(handler, frame.cube, currentSDF);
    bool hasSurface = isSdfSurface(currentSDF);
    bool contains = isSdfSolid(currentSDF);
    glm::vec3 previousNormal = node? node->vertex.normal : glm::vec3(0.0f);
    NodeOperationResult children[8];

    if(handler.check(frame.cube) == ContainmentType::Disjoint) {
        return NodeOperationResult(frame.cube);  // Skip this node
    }

    if (!isLeaf) {
        for (int i = 7; i >= 0; --i) {  
            OctreeNode * childNode = node ? node->getChildNode(i, &allocator, node->getBlock(&allocator)) : NULL;
            NodeOperationResult child = shape(handler, dirtyHandler, OctreeNodeFrame(childNode, i, frame.cube.getChild(i), frame.minSize, frame.level + 1, isSolid), chunk, simplifier, isAdd);
            children[i] = child;
            hasSurface |= child.hasSurface;
            contains &= child.contains;
        }
    }

    bool shouldClear = contains;
    if(shouldClear) {
        if(node != NULL) {
            node->clear(&allocator, frame.cube);
        }
    } else if(hasSurface && (isAdd ? !isSolid : isSolid)) {
        if(node == NULL) {
            node = allocator.allocateOctreeNode(frame.cube)->init(Vertex(frame.cube.getCenter()));   
        } 
        Vertex vertex = handler.getVertex(frame.cube, frame.cube.getCenter());
        vertex.normal = glm::normalize(previousNormal + (isAdd ? vertex.normal : (-vertex.normal)));
        node->vertex = vertex;

        for(int i =0 ; i < 8 ; ++i) {
            NodeOperationResult child = children[i];
            OctreeNode * childNode = child.node;

            if(childNode == NULL && child.contains) {
                childNode = allocator.allocateOctreeNode(child.cube)->init(Vertex(child.cube.getCenter()));
                childNode->setSolid(true);
                childNode->setSdf(child.sdf);
                childNode->setSimplification(0);
                childNode->setDirty(true);
            }

            if(childNode != NULL) {
                node->setChildNode(i, childNode, &allocator);
            }
        }
    }
    if(node!=NULL) {
        for(int i = 0; i < 8; ++i) {           
            if(isAdd) {
                node->sdf[i] = SDF::opUnion(currentSDF[i], node->sdf[i]);
            }
            else {
                node->sdf[i] = SDF::opSubtraction(currentSDF[i], node->sdf[i]);
            }
        }
        node->setSolid(contains);
        node->setSimplification(0);
        node->setDirty(true);
        if(node->id) {
            dirtyHandler.handle(node);
        }
    }

    return NodeOperationResult(frame.cube, node, hasSurface, contains, currentSDF);
}

void Octree::del(const ContainmentHandler &handler, const OctreeNodeDirtyHandler &dirtyHandler, float minSize, Simplifier &simplifier) {
    shape(handler, dirtyHandler, OctreeNodeFrame(root, -1, *this, minSize, 0, root->isSolid()), NULL, simplifier, false);
}

void Octree::iterate(IteratorHandler &handler) {
	BoundingCube cube(glm::vec3(getMinX(),getMinY(),getMinZ()),getLengthX());
	handler.iterate(OctreeNodeData(0, chunkSize, root, cube, NULL, &this->allocator));
}

void Octree::iterateFlat(IteratorHandler &handler) {
	BoundingCube cube(glm::vec3(getMinX(),getMinY(),getMinZ()),getLengthX());
    handler.iterateFlatIn(OctreeNodeData(0, chunkSize,root, cube, NULL, &this->allocator));
}


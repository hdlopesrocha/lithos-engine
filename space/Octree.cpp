#include "space.hpp"

//      6-----7
//     /|    /|
//    4z+---5 |
//    | 2y--+-3
//    |/    |/
//    0-----1x

static std::vector<glm::ivec4> tessOrder;
static std::vector<glm::ivec2> tessEdge;
static bool initialized = false;

Octree::Octree(BoundingCube minCube) : BoundingCube(minCube){
	this->root = allocator.nodeAllocator.allocate()->init(glm::vec3(minCube.getCenter()));
    this->dataId = 0;
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

        for(int i =0 ; i < 8 ; ++i) {
            OctreeNode* subNode = node->getChildNode(i, &allocator);
            if(subNode != NULL) {
                BoundingCube subCube = cube.getChild(i);
                bool cubeIntersects = subCube.intersects(c);
                if(cubeIntersects) {               
                    candidate = subNode;
                    candidateCube = subCube;
                    anyIntersection = true;
                    if(!candidate->isSolid) {
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

    return node->isSolid ? ContainmentType::Contains : ContainmentType::Intersects;
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
        if (node->isSolid) {
            break;
        }
		
		int i = getNodeIndex(pos, cube);
        if (i < 0) {
            return ContainmentType::Disjoint;
        }

   		OctreeNode* candidate = node->getChildNode(i, &allocator);
        if (candidate == NULL) {
            break;
        }

        cube = cube.getChild(i);
        node = candidate;
    }
    if (node == NULL){
		return ContainmentType::Disjoint;
	} 

    return node->isSolid ? ContainmentType::Contains : ContainmentType::Intersects;
}

OctreeNode* Octree::getNodeAt(const glm::vec3 &pos, int level, bool simplification) {
    OctreeNode* node = root;
    BoundingCube cube = *this;
	if(!contains(pos)) {
		return NULL;
	}

    for (; node && level > 0; --level) {
        if (simplification && node->simplified) {
            break;
        }
        int i = getNodeIndex(pos, cube);
        cube = cube.getChild(i);
        node = node->getChildNode(i, &allocator);
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
        if (simplification && node->simplified) {
            break;
        }
        int i = getNodeIndex(pos, cube);
        cube = cube.getChild(i);
        
        candidate = node->getChildNode(i, &allocator);
    }
    return node;
}



void Octree::getNodeNeighbors(OctreeNodeData &data, bool simplification, OctreeNode ** out, int direction, int initialIndex, int finalIndex) {
	for(int i=initialIndex; i < finalIndex; ++i) {
		glm::vec3 pos = data.cube.getCenter() + direction* data.cube.getLengthX() * Octree::getShift(i);
        out[i] = getNodeAt(pos, data.level, simplification);
	}
}

int Octree::getLevelAt(const glm::vec3 &pos, bool simplification) {
    OctreeNode* node = root;
    BoundingCube cube = *this;
	if(!contains(pos)) {
		return 0;
	}
    int level = 0;
    while (node) {
        if (simplification && node->simplified) {
            break;
        }
        int i = getNodeIndex(pos, cube);
        cube = cube.getChild(i);
        node = node->getChildNode(i, &allocator);
        ++level;
    }
    return level;
}

int Octree::getMaxLevel(OctreeNode *node, int level) {
    int l = level;
    for(int i=0; i < 8; ++i) {
        OctreeNode * n = node->getChildNode(i, &allocator);
        if(n!=NULL) {
            l = glm::max(l, getMaxLevel(n, level + 1));
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
        glm::vec3 pos = data.cube.getCenter() + direction* data.cube.getLengthX() * Octree::getShift(i);
        out[i] = getNodeAt(pos, data.level, simplification);
    }
    return out[i];
}

void Octree::handleQuadNodes(OctreeNodeData &data, OctreeNodeTriangleHandler * handler, bool simplification) {
	OctreeNode * neighbors[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	
    for(size_t k =0 ; k < tessOrder.size(); ++k) {
		glm::ivec2 edge = tessEdge[k];
		uint mask = data.node->mask;
		bool sign0 = (mask & (1 << edge[0])) != 0;
		bool sign1 = (mask & (1 << edge[1])) != 0;

		if(sign0 != sign1) {
			glm::ivec4 quad = tessOrder[k];
			OctreeNode * quads[4] = {NULL, NULL, NULL, NULL};
			for(int i =0; i<4 ; ++i) {
				OctreeNode * n = fetch(data, neighbors, quad[i]);
				quads[i] = (n != NULL && !n->isSolid) ? n : NULL;
                if(n == NULL) {
                    break;
                }
			} 

			handler->handle(quads[0],quads[2],quads[1],sign1);
			handler->handle(quads[0],quads[3],quads[2],sign1);
		}
	}
}

struct OctreeNodeFrame {
    OctreeNode* parent;
    int childIndex;
    BoundingCube cube;
    float minSize;
};

uint buildMask(const ContainmentHandler &handler, BoundingCube &cube) {
    const glm::vec3 min = cube.getMin();
    const float lengthX = cube.getLengthX();
    uint mask = 0;

    for (int i = 0; i < 8; ++i) {
        mask |= (uint)handler.contains(min + lengthX * Octree::getShift(i)) << i;
    }

    return mask;
}

void split(Octree * tree, OctreeNode * node, BoundingCube &cube, bool reverse) {
    Vertex vertex = node->vertex;
    Plane plane(vertex.normal, vertex.position);
	for(int i=0; i <8 ; ++i) {
        OctreeNode * child = node->getChildNode(i, &tree->allocator);
        if(child == NULL) {
            BoundingCube subCube = cube.getChild(i);
            if(plane.test(subCube) != (reverse ? ContainmentType::Contains : ContainmentType::Disjoint) ) {
                child = tree->allocator.nodeAllocator.allocate()->init(Vertex(subCube.getCenter(), vertex.normal, vertex.texCoord, vertex.brushIndex));
                child->isSolid = node->isSolid;
                child->mask = node->mask;
                node->setChildNode(i, child, &tree->allocator);
            }
        }
	}
}

void Octree::expand(const ContainmentHandler &handler) {
	while (true) {
		Vertex vertex(getCenter());
	    if (handler.isContained(*this)) {
	        break;
	    }
		glm::vec3 point = handler.getCenter();
	    unsigned int i = 7 - getNodeIndex(point, *this);

	    setMin(getMin() -  Octree::getShift(i) * getLengthX());
	    setLength(getLengthX()*2);

	    if(root != NULL && root->isLeaf()) {
	    	allocator.nodeAllocator.deallocate(root);
			root = NULL;
	    }
	    OctreeNode * newNode = allocator.nodeAllocator.allocate()->init(getCenter());
		newNode->setChildNode(i, root, &allocator);
	    root = newNode;
	}
}

void Octree::add(const ContainmentHandler &handler, const OctreeNodeDirtyHandler &dirtyHandler, float minSize) {
	expand(handler);	

    OctreeNode *node = root;
    BoundingCube &cube = *this; 
    std::stack<OctreeNodeFrame> stack;
    stack.push({ node, -1, cube, minSize});

    while (!stack.empty()) {
        OctreeNodeFrame frame = stack.top();
        stack.pop();

        OctreeNode* node = frame.childIndex < 0 ? frame.parent : frame.parent->getChildNode(frame.childIndex, &allocator);
        ContainmentType check = handler.check(frame.cube);
 
        if (check == ContainmentType::Disjoint) {
            continue;  // Skip this node
        }

        if (node == NULL) {
            node = allocator.nodeAllocator.allocate()->init(Vertex(frame.cube.getCenter()));
            if(frame.childIndex >= 0) {
                frame.parent->setChildNode(frame.childIndex, node, &allocator);
            }
        } else if (node->isSolid) {
            continue;  // No need to process further
        }

        if (check == ContainmentType::Intersects) {
            glm::vec3 previousNormal = node->vertex.normal;
            Vertex vertex = handler.getVertex(frame.cube, check, node->vertex.position);
            vertex.normal = glm::normalize(previousNormal + vertex.normal);
            node->vertex = vertex;
        }
        node->mask |= buildMask(handler, frame.cube);
        node->isSolid = check == ContainmentType::Contains;
        if(node->dataId) {
            dirtyHandler.handle(node->dataId);
        }
        bool isLeaf = frame.cube.getLengthX() <= frame.minSize;
        if (check == ContainmentType::Contains) {
            node->clear(&allocator);
        } else if (!node->isLeaf() || !isLeaf) {
            for (int i = 7; i >= 0; --i) {  
                BoundingCube subCube = frame.cube.getChild(i);
                stack.push({ node, i, subCube, frame.minSize });
            }
        }
    }
}

void Octree::del(const ContainmentHandler &handler, const OctreeNodeDirtyHandler &dirtyHandler, float minSize) {
    OctreeNode *node = root;
    BoundingCube &cube = *this; 
    std::stack<OctreeNodeFrame> stack;
    stack.push({ node, -1, cube, minSize });

    while (!stack.empty()) {
        OctreeNodeFrame frame = stack.top();
        stack.pop();

        OctreeNode* node = frame.childIndex < 0 ? frame.parent : frame.parent->getChildNode(frame.childIndex, &allocator);

        ContainmentType check = handler.check(frame.cube);
        if (check == ContainmentType::Disjoint) {
            continue;  // Skip this node
        }

        bool isContained = check == ContainmentType::Contains;
        bool isIntersecting = check == ContainmentType::Intersects;

        if (isContained) {
            if (node != NULL) {
                node->clear(&allocator);
                allocator.nodeAllocator.deallocate(node);
                if(frame.childIndex >= 0) {
                    frame.parent->setChildNode(frame.childIndex, NULL, &allocator);
                }
            }
            continue;
        }

        if (node != NULL) {
            bool isLeaf = frame.cube.getLengthX() <= frame.minSize;
            if (node->isSolid && isIntersecting && !isLeaf) {
                split(this, node, frame.cube, true);
            }

            if (isIntersecting) {
                glm::vec3 previousNormal = node->vertex.normal;
                Vertex vertex = handler.getVertex(frame.cube, check, node->vertex.position);
                vertex.normal = glm::normalize(previousNormal - vertex.normal);
                node->vertex = vertex;
            }

            node->mask &= buildMask(handler, frame.cube) ^ 0xff;
            node->isSolid = check == ContainmentType::Contains;
            if(node->dataId) {
                dirtyHandler.handle(node->dataId);
            }
            if (!isLeaf) {
                for (int i = 7; i >= 0; --i) { 
                    BoundingCube subCube = frame.cube.getChild(i);
                    stack.push({ node, i, subCube, frame.minSize });
                }
            }
        }
    }
}

void Octree::iterate(IteratorHandler &handler, float chunkSize) {
	BoundingCube cube(glm::vec3(getMinX(),getMinY(),getMinZ()),getLengthX());
	handler.iterate(OctreeNodeData(0, chunkSize, root, cube, NULL, &this->allocator));
}

void Octree::iterateFlat(IteratorHandler &handler, float chunkSize) {
	BoundingCube cube(glm::vec3(getMinX(),getMinY(),getMinZ()),getLengthX());
    handler.iterateFlatIn(OctreeNodeData(0, chunkSize,root, cube, NULL, &this->allocator));
}


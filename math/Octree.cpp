#include "math.hpp"

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
	this->minSize = minCube.getLengthX();
	this->root = new OctreeNode(glm::vec3(minSize*0.5));
	if(!initialized) {
		tessOrder.push_back(glm::ivec4(0,1,3,2));tessEdge.push_back(glm::ivec2(3,7));
		tessOrder.push_back(glm::ivec4(0,2,6,4));tessEdge.push_back(glm::ivec2(6,7));
		tessOrder.push_back(glm::ivec4(0,4,5,1));tessEdge.push_back(glm::ivec2(5,7));

		initialized = true;
	}
}

BoundingCube Octree::getChildCube(const BoundingCube &cube, int i) {
	float newLength = 0.5*cube.getLengthX();
    return BoundingCube(cube.getMin() + newLength * Octree::getShift(i), newLength);
}

BoundingCube Octree::getCube3(const BoundingCube &cube, int i) {
    return BoundingCube(cube.getMin() + cube.getLengthX() * Octree::getShift3(i), cube.getLengthX());
}

int getNodeIndex(const glm::vec3 &vec, const BoundingCube &cube) {
	glm::vec3 c = cube.getCenter();
    return (vec.x >= c.x ? 4 : 0) + (vec.y >= c.y ? 2 : 0) + (vec.z >= c.z ? 1 : 0);
}

ContainmentType Octree::contains(const AbstractBoundingBox &c) {
    OctreeNode* node = root;
    BoundingCube cube = *this;
    
    // If c is not completely within the overall cube, it's disjoint.
    if (!cube.contains(c))
        return ContainmentType::Disjoint;
    
    while (node) {
        // If we hit a node that is fully solid, it must be a leaf.
        if (node->solid == ContainmentType::Contains)
            return ContainmentType::Contains;
        
        // Determine the candidate child based on c's center.
        int idx = getNodeIndex(c.getCenter(), cube);
        if (idx < 0)
            return ContainmentType::Disjoint;
        
        // Get the bounding cube for the candidate child.
        BoundingCube childCube = getChildCube(cube, idx);
        
        // If c is not fully inside this child cube, then c spans multiple children.
        if (!childCube.contains(c))
            return ContainmentType::Intersects;
        
        // If the child does not exist, we cannot descend further,
        // so the cube is not completely inside a fully solid region.
        if (node->children[idx] == nullptr)
            return ContainmentType::Intersects;
        
        // Descend into the candidate child.
        node = node->children[idx];
        cube = childCube;
    }
    
    return ContainmentType::Intersects;
}


ContainmentType Octree::contains(const glm::vec3 &pos) {
    OctreeNode* node = root;
    BoundingCube cube = *this;
    int level = getHeight(*this);

    for (; node && level > 0; --level) {
        bool testResult = cube.contains(pos);

        // If completely outside, return Disjoint immediately
        if (!testResult) {
            return ContainmentType::Disjoint;
        }

        // If the node is marked as solid and the cube is not Disjoint, return Contains
        if (node->solid == ContainmentType::Contains) {
            break;
        }
		
		int i = getNodeIndex(pos, cube);
        if (i < 0) {
            return ContainmentType::Disjoint;
        }

   		OctreeNode* candidate = node->children[i];
        if (candidate == NULL) {
            return node->solid;
        }

        cube = getChildCube(cube, i);
        node = candidate;
    }
    if (node == NULL){
		return ContainmentType::Disjoint;
	} 

    return node->solid;
}

OctreeNode* Octree::getNodeAt(const glm::vec3 &pos, int level, int simplification) {
    OctreeNode* node = root;
    BoundingCube cube = *this;
	if(!contains(pos)) {
		return NULL;
	}

    for (; node && level > 0; --level) {
        if (simplification && node->simplification == simplification) {
            return node;
        }
        int i = getNodeIndex(pos, cube);
        cube = getChildCube(cube, i);
        node = node->children[i];
    }
    return level == 0 ? node : NULL;
}

int Octree::getHeight(const BoundingCube &cube){
	float r = glm::log2(cube.getLengthX() / minSize);
	return r >= 0  ? (int) glm::floor(r) : -1;
}

void Octree::getNodeNeighbors(const BoundingCube &cube, int level, int simplification, int direction, OctreeNode ** out, int initialIndex, int finalIndex) {
	// Get corners
	for(int i=initialIndex; i < finalIndex; ++i) {
		glm::vec3 pos = cube.getCenter() + direction * cube.getLengthX() * Octree::getShift(i);
		OctreeNode * n = getNodeAt(pos, level, simplification);
		out[i] = n;
	}
}

void Octree::handleQuadNodes(const BoundingCube &cube, int level,OctreeNode &node, OctreeNodeTriangleHandler * handler) {
	OctreeNode * neighbors[8];
	getNodeNeighbors(cube, level, 0, 1, neighbors, 0, 8);

	
	for(int k =0 ; k < tessOrder.size(); ++k) {
		glm::ivec2 edge = tessEdge[k];
		uint mask = node.mask;
		bool sign0 = (mask & (1 << edge[0])) != 0;
		bool sign1 = (mask & (1 << edge[1])) != 0;

		if(sign0 != sign1) {
			glm::ivec4 quad = tessOrder[k];
			OctreeNode * quads[4];
			for(int i =0; i<4 ; ++i){
				OctreeNode * n = neighbors[quad[i]];
				quads[i] = (n != NULL && n->solid == ContainmentType::Intersects) ? n : NULL;
			}

			handler->handle(quads[0],quads[2],quads[1],sign1);
			handler->handle(quads[0],quads[3],quads[2],sign1);
		} 
	}
}

struct OctreeNodeFrame {
    OctreeNode** nodePtr;
    BoundingCube cube;
    int level;
    int height;
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

void split(OctreeNode * node, BoundingCube &cube) {
	for(int i=0; i <8 ; ++i) {
		BoundingCube subCube = Octree::getChildCube(cube,i);
		node->children[i] = new OctreeNode(subCube.getCenter());
		node->children[i]->solid = node->solid;
		node->children[i]->mask = node->mask;
	}	
}

void Octree::expand(const ContainmentHandler &handler) {
	while (true) {
		Vertex vertex(getCenter());
		ContainmentType cont = handler.check(*this);
	    if (handler.isContained(*this)) {
	        break;
	    }
		glm::vec3 point = handler.getCenter();
	    unsigned int i = 7 - getNodeIndex(point, *this);

	    setMin(getMin() -  Octree::getShift(i) * getLengthX());
	    setLength(getLengthX()*2);

	    if(root != NULL && root->isEmpty()) {
	    	delete root;
			root = NULL;
	    }
	    OctreeNode * newNode = new OctreeNode(getCenter());
		newNode->setChild(i, root);
	    root = newNode;
	}
}

OctreeNode* addAux(Octree &tree, const ContainmentHandler &handler, OctreeNode *node, BoundingCube &cube, int level) {
    std::stack<OctreeNodeFrame> stack;
    stack.push({ &node, cube, level , tree.getHeight(cube)});

    while (!stack.empty()) {
        OctreeNodeFrame frame = stack.top();
        stack.pop();

        OctreeNode** nodePtr = frame.nodePtr;

        ContainmentType check = handler.check(frame.cube);
        if (check == ContainmentType::Disjoint) {
            continue;  // Skip this node
        }

        if (*nodePtr == NULL) {
            *nodePtr = new OctreeNode(Vertex(frame.cube.getCenter()));
        } else if ((*nodePtr)->solid == ContainmentType::Contains) {
            continue;  // No need to process further
        }

        if (check == ContainmentType::Intersects) {
            (*nodePtr)->vertex = handler.getVertex(frame.cube, check, (*nodePtr)->vertex.position);
        }
        (*nodePtr)->mask |= buildMask(handler, frame.cube);
        (*nodePtr)->solid = check;

        if (check == ContainmentType::Contains) {
            (*nodePtr)->clear();
        } else if (frame.height != 0) {
            for (int i = 7; i >= 0; --i) {  // Push children in reverse order to maintain order
                BoundingCube subCube = Octree::getChildCube(frame.cube, i);
                stack.push({ &((*nodePtr)->children[i]), subCube, frame.level + 1 , frame.height -1});
            }
        }
    }

    return node;
}


OctreeNode* delAux(Octree &tree, const ContainmentHandler &handler, OctreeNode *node, BoundingCube &cube, int level) {
    std::stack<OctreeNodeFrame> stack;
    stack.push({ &node, cube, level , tree.getHeight(cube) });

    while (!stack.empty()) {
        OctreeNodeFrame frame = stack.top();
        stack.pop();

        OctreeNode** nodePtr = frame.nodePtr;

        ContainmentType check = handler.check(frame.cube);
        if (check == ContainmentType::Disjoint) {
            continue;  // Skip this node
        }

        bool isContained = check == ContainmentType::Contains;
        bool isIntersecting = check == ContainmentType::Intersects;

        if (isContained) {
            if (*nodePtr != NULL) {
                (*nodePtr)->clear();
                delete *nodePtr;
                *nodePtr = NULL;
            }
            continue;
        }

        if (*nodePtr != NULL) {
            if ((*nodePtr)->solid == ContainmentType::Contains && isIntersecting && frame.height != 0) {
                split(*nodePtr, frame.cube);
            }

            if (isIntersecting) {
                glm::vec3 previousNormal = (*nodePtr)->vertex.normal;
                (*nodePtr)->vertex = handler.getVertex(frame.cube, check, (*nodePtr)->vertex.position);
                (*nodePtr)->vertex.normal = glm::normalize(previousNormal - (*nodePtr)->vertex.normal);
            }

            (*nodePtr)->mask &= buildMask(handler, frame.cube) ^ 0xff;
            (*nodePtr)->solid = check;

            if (frame.height != 0) {
                for (int i = 7; i >= 0; --i) {  // Push children in reverse order
                    BoundingCube subCube = Octree::getChildCube(frame.cube, i);
                    stack.push({ &((*nodePtr)->children[i]), subCube, frame.level + 1 , frame.height -1});
                }
            }
        }
    }

    return node;
}

void Octree::add(const ContainmentHandler &handler) {
	expand(handler);	
	root = addAux(*this, handler, root, *this, 0);
}

void Octree::del(const ContainmentHandler &handler) {
	root = delAux(*this, handler, root, *this, 0);
}


void Octree::iterate(IteratorHandler &handler, int geometryLevel) {
	BoundingCube cube(glm::vec3(getMinX(),getMinY(),getMinZ()),getLengthX());
	handler.iterate(IteratorData(0, getHeight(cube), geometryLevel, root, cube, NULL));
}

void Octree::iterateFlat(IteratorHandler &handler, int geometryLevel) {
	BoundingCube cube(glm::vec3(getMinX(),getMinY(),getMinZ()),getLengthX());
	handler.iterateFlatIn(IteratorData(0, getHeight(cube), geometryLevel,root, cube, NULL));
}


glm::vec3 Octree::getShift(int i) {
	return glm::vec3( ((i >> 2) % 2) , ((i >> 1) % 2) , ((i >> 0) % 2));
}

glm::vec3 Octree::getShift3(int i) {
	return glm::vec3( ((i / 9) % 3)-1 , ((i /3) % 3)-1 , ((i) % 3)-1);
}
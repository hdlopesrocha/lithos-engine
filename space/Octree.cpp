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
	this->root = new OctreeNode(glm::vec3(minCube.getCenter()), false);
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

ContainmentType containsRecursive(OctreeNode* node, const BoundingCube& cube, const AbstractBoundingBox& c) {
    if (!cube.intersects(c))
        return ContainmentType::Disjoint;

    if (!node)
        return ContainmentType::Disjoint;

    if (node->solid == ContainmentType::Contains) {
        if (cube.contains(c))
            return ContainmentType::Contains;
        else
            return ContainmentType::Intersects;
    }

    if (node->solid == ContainmentType::Disjoint)
        return ContainmentType::Disjoint;

    bool allContained = true;
    bool anyIntersecting = false;

    for (int i = 0; i < 8; ++i) {
        BoundingCube childCube = cube.getChild(i);

        if (!childCube.intersects(c))
            continue;

        ContainmentType result = containsRecursive(node->children[i], childCube, c);

        if (result == ContainmentType::Intersects)
            return ContainmentType::Intersects;
        else if (result == ContainmentType::Disjoint)
            allContained = false;
        else if (result == ContainmentType::Contains)
            anyIntersecting = true;
    }

    if (allContained && anyIntersecting)
        return ContainmentType::Contains;
    else if (anyIntersecting)
        return ContainmentType::Intersects;
    else
        return ContainmentType::Disjoint;
}


ContainmentType Octree::contains(const AbstractBoundingBox &c) {
    return containsRecursive(root, *this, c);
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

        cube = cube.getChild(i);
        node = candidate;
    }
    if (node == NULL){
		return ContainmentType::Disjoint;
	} 

    return node->solid;
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
        node = node->children[i];
    }
    return node;
}


void Octree::getNodeNeighbors(OctreeNodeData &data, bool simplification, OctreeNode ** out, int direction, int initialIndex, int finalIndex) {
	for(int i=initialIndex; i < finalIndex; ++i) {
		glm::vec3 pos = data.cube.getCenter() + direction* data.cube.getLengthX() * Octree::getShift(i);
        out[i] = getNodeAt(pos, data.level, simplification);
	}
}

void Octree::handleQuadNodes(OctreeNodeData &data, OctreeNodeTriangleHandler * handler, bool simplification) {
	OctreeNode * neighbors[8];
    
	getNodeNeighbors(data, simplification, neighbors, 1, 0, 8);

	for(size_t k =0 ; k < tessOrder.size(); ++k) {
		glm::ivec2 edge = tessEdge[k];
		uint mask = data.node->mask;
		bool sign0 = (mask & (1 << edge[0])) != 0;
		bool sign1 = (mask & (1 << edge[1])) != 0;

		if(sign0 != sign1) {
			glm::ivec4 quad = tessOrder[k];
			OctreeNode * quads[4];
			for(int i =0; i<4 ; ++i) {
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

void split(OctreeNode * node, BoundingCube &cube, float minSize) {
    Vertex vertex = node->vertex;

	for(int i=0; i <8 ; ++i) {
		BoundingCube subCube = cube.getChild(i);
        bool isLeaf = subCube.getLengthX() <= minSize;
        node->children[i] = new OctreeNode(Vertex(subCube.getCenter(), glm::vec3(0), glm::vec2(0.0), vertex.brushIndex), isLeaf);
		node->children[i]->solid = node->solid;
		node->children[i]->mask = node->mask;
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

	    if(root != NULL && root->isEmpty()) {
	    	delete root;
			root = NULL;
	    }
	    OctreeNode * newNode = new OctreeNode(getCenter(), false);
		newNode->setChild(i, root);
	    root = newNode;
	}
}

void Octree::add(const ContainmentHandler &handler, float minSize) {
	expand(handler);	

    OctreeNode *node = root;
    BoundingCube &cube = *this; 
    int level = 0;

    std::stack<OctreeNodeFrame> stack;
    stack.push({ &node, cube, level});

    while (!stack.empty()) {
        OctreeNodeFrame frame = stack.top();
        stack.pop();

        OctreeNode** nodePtr = frame.nodePtr;
        ContainmentType check = handler.check(frame.cube);
        bool isLeaf = frame.cube.getLengthX() <= minSize;

        if (check == ContainmentType::Disjoint) {
            continue;  // Skip this node
        }

        if (*nodePtr == NULL) {
            *nodePtr = new OctreeNode(Vertex(frame.cube.getCenter()), isLeaf);
        } else if ((*nodePtr)->solid == ContainmentType::Contains) {
            continue;  // No need to process further
        }

        if (check == ContainmentType::Intersects) {
            glm::vec3 previousNormal = (*nodePtr)->vertex.normal;
            Vertex vertex = handler.getVertex(frame.cube, check, (*nodePtr)->vertex.position);
            vertex.normal = glm::normalize(previousNormal + vertex.normal);
            (*nodePtr)->vertex = vertex;
        }
        (*nodePtr)->mask |= buildMask(handler, frame.cube);
        (*nodePtr)->solid = check;

        if (check == ContainmentType::Contains) {
            (*nodePtr)->clear();
        } else if (!isLeaf) {
            for (int i = 7; i >= 0; --i) {  // Push children in reverse order to maintain order
                BoundingCube subCube = frame.cube.getChild(i);
                stack.push({ &((*nodePtr)->children[i]), subCube, frame.level + 1 });
            }
        }
    }
}

void Octree::del(const ContainmentHandler &handler, float minSize) {
    OctreeNode *node = root;
    BoundingCube &cube = *this; 
    int level = 0;


    std::stack<OctreeNodeFrame> stack;
    stack.push({ &node, cube, level });

    while (!stack.empty()) {
        OctreeNodeFrame frame = stack.top();
        stack.pop();

        OctreeNode** nodePtr = frame.nodePtr;
        bool isLeaf = frame.cube.getLengthX() <= minSize;

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
            if ((*nodePtr)->solid == ContainmentType::Contains && isIntersecting && frame.cube.getLengthX() > minSize) {
                split(*nodePtr, frame.cube, minSize);
            }

            if (isIntersecting) {
                glm::vec3 previousNormal = (*nodePtr)->vertex.normal;
                Vertex vertex = handler.getVertex(frame.cube, check, (*nodePtr)->vertex.position);
                vertex.normal = glm::normalize(previousNormal - vertex.normal);
                (*nodePtr)->vertex = vertex;
            }

            (*nodePtr)->mask &= buildMask(handler, frame.cube) ^ 0xff;
            (*nodePtr)->solid = check;

            if (!isLeaf) {
                for (int i = 7; i >= 0; --i) {  // Push children in reverse order
                    BoundingCube subCube = frame.cube.getChild(i);
                    stack.push({ &((*nodePtr)->children[i]), subCube, frame.level + 1 });
                }
            }
        }
    }
}


void Octree::iterate(IteratorHandler &handler, float chunkSize) {
	BoundingCube cube(glm::vec3(getMinX(),getMinY(),getMinZ()),getLengthX());
	handler.iterate(OctreeNodeData(0, chunkSize, root, cube, NULL));
}

void Octree::iterateFlat(IteratorHandler &handler, float chunkSize) {
	BoundingCube cube(glm::vec3(getMinX(),getMinY(),getMinZ()),getLengthX());
    handler.iterateFlatIn(OctreeNodeData(0, chunkSize,root, cube, NULL));
}


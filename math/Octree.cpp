#include "math.hpp"

static std::vector<glm::ivec4> tessOrder;
static std::vector<glm::ivec2> tessEdge;
static bool initialized = false;

Octree::Octree(BoundingCube minCube) : BoundingCube(minCube){
	this->minSize = minCube.getLength();
	this->root = new OctreeNode(glm::vec3(minSize*0.5));
	if(!initialized) {
		tessOrder.push_back(glm::ivec4(0,1,3,2));tessEdge.push_back(glm::ivec2(3,7));
		tessOrder.push_back(glm::ivec4(0,2,6,4));tessEdge.push_back(glm::ivec2(6,7));
		tessOrder.push_back(glm::ivec4(0,4,5,1));tessEdge.push_back(glm::ivec2(5,7));

		initialized = true;
	}
}

BoundingCube Octree::getChildCube(BoundingCube cube, int i) {
	float newLength = 0.5*cube.getLength();
    return BoundingCube(cube.getMin() + newLength * Octree::getShift(i), newLength);
}

BoundingCube Octree::getCube3(BoundingCube cube, int i) {
    return BoundingCube(cube.getMin() + cube.getLength() * Octree::getShift3(i), cube.getLength());
}

int getNodeIndex(glm::vec3 vec, BoundingCube cube, bool checkBounds) {
	if(checkBounds && !cube.contains(vec)) {
		return -1;
	}
	glm::vec3 diff = (vec - cube.getMin()) / cube.getLength();
	int px = Math::clamp(round(diff[0]), 0, 1);
	int py = Math::clamp(round(diff[1]), 0, 1);
	int pz = Math::clamp(round(diff[2]), 0, 1);
	return px * 4 + py * 2 + pz;
}

OctreeNode * Octree::getNodeAt(glm::vec3 pos, int level, int simplification) {
	OctreeNode * node = root;
	BoundingCube cube = *this;

	while(node!= NULL && level>0){
		if(simplification && node->simplification == simplification) {
			return node;
		}
		unsigned int i = getNodeIndex(pos, cube, true);
	  	if(i == -1) {
	  		return NULL;
	  	}
	    
	    cube = getChildCube(cube, i);
		OctreeNode * candidate = node->children[i];
	
		node = candidate;
		--level;
	}

	return level == 0 ? node : NULL;
}

void Octree::expand(ContainmentHandler * handler) {
	while (true) {
		Vertex vertex(getCenter());
		ContainmentType cont = handler->check(*this);
	    if (handler->isContained(*this)) {
	        break;
	    }
	
	    unsigned int i = 7 - getNodeIndex(handler->getCenter(), *this, false);

	    setMin(getMin() -  Octree::getShift(i) * getLength());
	    setLength(getLength()*2);

	    OctreeNode * newNode = new OctreeNode(getCenter());
	    if(root->isEmpty()) {
	    	delete root;
	    }
	    else {
			newNode->setChild(i, root);
	    }
	    root = newNode;
	}
}

int Octree::getHeight(BoundingCube cube){
	float r = glm::log2(cube.getLength() / minSize);
	return r >= 0  ? (int) glm::floor(r) : -1;
}

void Octree::getNeighbors(BoundingCube cube, int level, OctreeNode ** out) {
	// Get corners
	//corners.push_back(node);
	for(int i=0; i < 27; ++i) {
		glm::vec3 s = Octree::getShift3(i);
		//std::cout << s.x << "," << s.y << "," << s.z << std::endl;
		glm::vec3 pos = cube.getCenter() + cube.getLength() * s;
		OctreeNode * n = getNodeAt(pos,level, 0);
		out[i] = n;
	}
}

void Octree::getNodeNeighbors(BoundingCube cube, int level, int simplification, int direction, OctreeNode ** out, int initialIndex, int finalIndex) {
	// Get corners
	//corners.push_back(node);
	for(int i=initialIndex; i < finalIndex; ++i) {
		glm::vec3 pos = cube.getCenter() + direction * cube.getLength() * Octree::getShift(i);
		OctreeNode * n = getNodeAt(pos, level, simplification);
		out[i] = n;
	}
}

void Octree::handleQuadNodes(OctreeNode * node,  OctreeNode** corners, OctreeNodeTriangleHandler * handler) {
	for(int k =0 ; k < tessOrder.size(); ++k) {
		glm::ivec2 edge = tessEdge[k];
		uint mask = node->mask;
		bool sign0 = (mask & (1 << edge[0])) != 0;
		bool sign1 = (mask & (1 << edge[1])) != 0;

		if(sign0 != sign1) {
			glm::ivec4 quad = tessOrder[k];
			OctreeNode * quads[4];
			for(int i =0; i<4 ; ++i){
				OctreeNode * n = corners[quad[i]];
				quads[i] = (n != NULL && n->solid == ContainmentType::Intersects) ? n : NULL;
			}
			handler->handle(quads[0],quads[2],quads[1],sign1);
			handler->handle(quads[0],quads[3],quads[2],sign1);
		} 
	}
}


uint buildMask(ContainmentHandler * handler, BoundingCube cube) {
	float d[8];
	uint mask = 0x00;
	for(int i=0 ; i < 8 ; ++i) {
		glm::vec3 p = cube.getMin() + cube.getLength()*Octree::getShift(i);
		bool contains = handler->contains(p);
		mask |= contains ? (1 << i) : 0;
	}
	return mask;
}

OctreeNode * addAux(Octree * tree, ContainmentHandler * handler, OctreeNode * node, BoundingCube cube, int level) {
	int height = tree->getHeight(cube);
	ContainmentType check = handler->check(cube);

	if(check == ContainmentType::Disjoint) {
		return node;
	}

	if(node == NULL) {
		Vertex vertex(cube.getCenter());
		node = new OctreeNode(vertex);
	}
	else if(node->solid == ContainmentType::Contains) {
		return node;
	}

	if(check == ContainmentType::Intersects) {
		node->vertex = handler->getVertex(cube, check);
	}
	node->mask |= buildMask(handler, cube);
	node->solid = check;
	
	if(check == ContainmentType::Contains) {
		node->clear();
	}
	else if(height != 0) {
		for(int i=0; i <8 ; ++i) {
			BoundingCube subCube = Octree::getChildCube(cube,i);
			node->children[i] = addAux(tree, handler, node->children[i], subCube, level +1);
		}
	}
	return node;
}

void split(OctreeNode * node, BoundingCube cube) {
	for(int i=0; i <8 ; ++i) {
		BoundingCube subCube = Octree::getChildCube(cube,i);
		node->children[i] = new OctreeNode(subCube.getCenter());
		node->children[i]->solid = node->solid;
		node->children[i]->mask = node->mask;
	}	
}

OctreeNode * delAux(Octree * tree,  ContainmentHandler * handler, OctreeNode * node, BoundingCube cube, int level) {
	ContainmentType check = handler->check(cube);

	if(check != ContainmentType::Disjoint) {
		bool height = tree->getHeight(cube);
		bool isContained = check == ContainmentType::Contains;
		bool isIntersecting = check == ContainmentType::Intersects;

		// Any full containment results in cleaning
		if(isContained) {
			if(node != NULL ) {
				node->clear();
				delete node;
			}
			return NULL;
		}
		if(node!= NULL) {
			// TODO: problem here
			if(node->solid == ContainmentType::Contains && isIntersecting && height != 0) {
				split(node, cube);
			}

			if(node->solid != ContainmentType::Intersects && isIntersecting) {
				node->vertex = handler->getVertex(cube, check);
				node->vertex.normal = -node->vertex.normal;
			}

			node->mask &= buildMask(handler, cube) ^ 0xff; 
			node->solid = check;

			if(height != 0) {
				for(int i=0; i <8 ; ++i) {
					BoundingCube subCube = Octree::getChildCube(cube,i);
					node->children[i] = delAux(tree, handler, node->children[i], subCube, level +1);
				}	
			}
		} 
	}
	return node;
}

void Octree::add(ContainmentHandler * handler) {
	expand(handler);	
	root = addAux(this, handler, root, *this, 0);
}

void Octree::del(ContainmentHandler * handler) {
	root = delAux(this, handler, root, *this, 0);
}

void iterateAux(IteratorHandler * handler, int level, OctreeNode * node, BoundingCube cube, void * context) {
	handler->iterate(level, node, cube, context);
}

void Octree::iterate(IteratorHandler * handler) {
	BoundingCube cube(glm::vec3(getMinX(),getMinY(),getMinZ()),getLength());
	iterateAux(handler, 0, root, cube, NULL);
}

glm::vec3 Octree::getShift(int i) {
	return glm::vec3( ((i >> 2) % 2) , ((i >> 1) % 2) , ((i >> 0) % 2));
}

glm::vec3 Octree::getShift3(int i) {
	return glm::vec3( ((i / 9) % 3)-1 , ((i /3) % 3)-1 , ((i) % 3)-1);
}
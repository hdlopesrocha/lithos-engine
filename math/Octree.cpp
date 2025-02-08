#include "math.hpp"


Octree::Octree(float minSize) : BoundingCube(glm::vec3(0,0,0), minSize){
	this->root = new OctreeNode(glm::vec3(minSize*0.5));
	this->minSize = minSize;
}

BoundingCube Octree::getChildCube(BoundingCube cube, int i) {
	float newLength = 0.5*cube.getLength();
    return BoundingCube(cube.getMin() + newLength * Octree::getShift(i), newLength);
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

OctreeNode * Octree::getNodeAt(glm::vec3 pos, int level, bool simplify) {
	OctreeNode * node = root;
	BoundingCube cube = *this;

	while(node!= NULL && level>0){
		if(simplify && node->simplified) {
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

std::vector<OctreeNode*> Octree::getNeighbors(BoundingCube cube, int level) {
	std::vector<OctreeNode*> corners;
	// Get corners
	//corners.push_back(node);
	for(int i=0; i < 27; ++i) {
		glm::vec3 s = Octree::getShift3(i);
		//std::cout << s.x << "," << s.y << "," << s.z << std::endl;
		glm::vec3 pos = cube.getCenter() + cube.getLength() * s;
		OctreeNode * n = getNodeAt(pos,level, false);
		corners.push_back(n);
	}
	return corners;
}

void Octree::getNodeCorners(BoundingCube cube, int level, bool simplify, int direction, OctreeNode ** out) {
	// Get corners
	//corners.push_back(node);
	for(int i=0; i < 8; ++i) {
		glm::vec3 pos = cube.getCenter() + direction * cube.getLength() * Octree::getShift(i);
		OctreeNode * n = getNodeAt(pos,level, simplify);
		out[i] = n;
	}
}

std::vector<OctreeNode*> Octree::getQuadNodes(OctreeNode** corners, glm::ivec4 quad) {
	std::vector<OctreeNode*> result;
	for(int i =0; i<4 ; ++i){
		OctreeNode * n = corners[quad[i]];
		if(n != NULL && n->solid == ContainmentType::Intersects) {
			result.push_back(n);
		} else {
			break;
		}
	}
	return result;
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
		node->simplified = false;
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
				node->simplified = false;
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

void saveAux(std::ofstream * myfile, OctreeNode * node) {
	if(node!= NULL) {
		Vertex vertex = node->vertex;
		*myfile << "\n{";
		*myfile << "\"v\":" << node->vertex.toString() << ",";
		*myfile << "\"m\":" << (uint) node->mask;
		if(node->children != NULL) {
			for(int i=0; i <8 ; ++i) {
				OctreeNode * c = node->children[i];
				if(c != NULL) {
					*myfile << ",";
					*myfile << "\"" << i << "\":";
					saveAux(myfile, c);
				}
			}
		}
		*myfile << "}";
	}else {
	  	*myfile << "null";
	}
}

void Octree::save(std::string filename) {
	std::ofstream myfile;
	myfile.open (filename);
	myfile << "{";
	myfile << "\"c\":[" << getMin()[0] << "," << getMin()[1] << "," << getMin()[2] << "],";
	myfile << "\"m\":" << minSize << ",";
	myfile << "\"l\":" << getLength() << ",";
	myfile << "\"r\":";
	saveAux(&myfile, root);
	myfile << "\n}";
	myfile.close();
}

glm::vec3 Octree::getShift(int i) {
	return glm::vec3( ((i >> 2) % 2) , ((i >> 1) % 2) , ((i >> 0) % 2));
}

glm::vec3 Octree::getShift3(int i) {
	return glm::vec3( ((i / 9) % 3)-1 , ((i /3) % 3)-1 , ((i) % 3)-1);
}
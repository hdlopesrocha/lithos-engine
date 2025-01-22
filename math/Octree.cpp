#include "math.hpp"


Octree::Octree(float minSize, int geometryLevel) : BoundingCube(glm::vec3(0,0,0), minSize){
	this->root = new OctreeNode(glm::vec3(minSize*0.5));
	this->minSize = minSize;
	this->geometryLevel = geometryLevel;
}

BoundingCube getChildCube(BoundingCube cube, int i) {
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

OctreeNode * Octree::getNodeAt(glm::vec3 pos, int level) {
	OctreeNode * node = root;
	BoundingCube cube = *this;

	while(node!= NULL && level>0){
	    unsigned int i = getNodeIndex(pos, cube, true);
	  	if(i == -1) {
	  		return NULL;
	  	}
	    /*if(node->children[i] == NULL){
	    	break;
	    }*/
	    
	    cube = getChildCube(cube, i);
		OctreeNode * candidate = node->children[i];
		node = candidate;
		--level;
	}

	return level == 0 ? node : NULL;
}

bool isEmpty(OctreeNode * node){
	for(int i=0; i < 8 ; ++i){
		if(node->children[i] != NULL) {
			return false;
		}
	}
	return true;
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
	    if(isEmpty(root)) {
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

OctreeNode * addAux(Octree * tree, ContainmentHandler * handler, OctreeNode * node, BoundingCube cube) {
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
	node->solid = check;
	uint mask = buildMask(handler, cube);
	node->mask |= mask;
	
	if(check == ContainmentType::Contains) {
		node->clear();
	}
	else if(height != 0) {
		for(int i=0; i <8 ; ++i) {
			BoundingCube subCube = getChildCube(cube,i);
			node->children[i] = addAux(tree, handler, node->children[i], subCube);
		}
	}
	return node;
}

void split(OctreeNode * node, BoundingCube cube, ContainmentType solid) {
	for(int i=0; i <8 ; ++i) {
		BoundingCube subCube = getChildCube(cube,i);
		node->children[i] = new OctreeNode(subCube.getCenter());
		// TODO: Confirm
		node->children[i]->solid = solid;
		node->children[i]->mask = 0xff;
	}	
}

OctreeNode * delAux(Octree * tree,  ContainmentHandler * handler, OctreeNode * node, BoundingCube cube) {
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
			if(node->solid == ContainmentType::Contains && height != 0) {
				split(node, cube, ContainmentType::Contains);
			}

			if(node->solid != ContainmentType::Intersects && isIntersecting) {
				node->vertex = handler->getVertex(cube, check);
				node->vertex.normal = -node->vertex.normal;

			}

			node->mask &= buildMask(handler, cube) ^ 0xff; 
			node->solid = check;
			
			if(height != 0) {
				for(int i=0; i <8 ; ++i) {
					BoundingCube subCube = getChildCube(cube,i);
					node->children[i] = delAux(tree, handler, node->children[i], subCube);
				}	
			}
		} 
	}
	return node;
}

void Octree::add(ContainmentHandler * handler) {
	expand(handler);	
	root = addAux(this, handler, root, *this);
}

void Octree::del(ContainmentHandler * handler) {
	root = delAux(this, handler, root, *this);
}

void iterateAux(IteratorHandler * handler, int level, OctreeNode * node, BoundingCube cube, void * context) {
	if(node != NULL) {
		if(handler->test(level,node, cube, context)) {
			context = handler->before(level,node, cube, context);
			for(int i=0; i <8 ; ++i) {
				iterateAux(handler, level+1,node->children[i], getChildCube(cube,i), context);
			}
			handler->after(level,node, cube, context);
		}
	}
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
		*myfile << "\"s\":" << (int) node->solid;
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
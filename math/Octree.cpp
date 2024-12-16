#include "math.hpp"


Octree::Octree(float minSize) : BoundingCube(glm::vec3(0,0,0), minSize){
	this->root = new OctreeNode(glm::vec3(minSize*0.5));
	this->minSize = minSize;
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
	    if(candidate == NULL && node->solid == ContainmentType::Contains) {
			return node;
		}	
		else {		
			node = candidate;
			--level;
		}
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
		ContainmentType cont = handler->check(*this, &vertex);
	    if (cont == ContainmentType::IsContained) {
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

int canSplit(BoundingCube cube, float minSize){
	float r = glm::log2(cube.getLength() / minSize);
	return r >= 0  ? (int) glm::floor(r) : -1;
}


OctreeNode * addAux(ContainmentHandler * handler, OctreeNode * node, BoundingCube cube, float minSize) {
	int height = !canSplit(cube, minSize);
	Vertex vertex;
	ContainmentType check = handler->check(cube, &vertex);
	
	if(check == ContainmentType::Disjoint) {
		return node;
	}

	if(node == NULL) {
		node = new OctreeNode(vertex);
	}
	else if(node->solid == ContainmentType::Contains) {
		return node;
	}


	if(check == ContainmentType::Intersects) {
		node->vertex = vertex;
	}
	node->height = height ? 0 : 1;
	node->solid = check;
	
	if(check == ContainmentType::Contains) {
		node->clear();
	}
	else if(height == 0) {
		for(int i=0; i <8 ; ++i) {
			BoundingCube subCube = getChildCube(cube,i);
			node->children[i] = addAux(handler, node->children[i], subCube, minSize);
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
	}	
}


OctreeNode * delAux(ContainmentHandler * handler, OctreeNode * node, BoundingCube cube, float minSize) {
	Vertex vertex;
	ContainmentType check = handler->check(cube, &vertex);

	bool height = canSplit(cube, minSize);

	if(check != ContainmentType::Disjoint) {
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
				node->vertex = vertex;
				node->vertex.normal = -vertex.normal;
			}

			node->solid = check;
		    node->height = height;

			if(height != 0) {
				for(int i=0; i <8 ; ++i) {
					BoundingCube subCube = getChildCube(cube,i);
					node->children[i] = delAux(handler, node->children[i], subCube, minSize);
				}	
			}
		} 
	}
	return node;
}

void Octree::add(ContainmentHandler * handler) {
	expand(handler);	
	root = addAux(handler, root, *this, minSize);
}

void Octree::del(ContainmentHandler * handler) {
	root = delAux(handler, root, *this, minSize);
}

void iterateAux(IteratorHandler * handler, int level, OctreeNode * node, BoundingCube cube) {
	if(node != NULL) {
		if(handler->iterate(level,node, cube)) {
			for(int i=0; i <8 ; ++i) {
				iterateAux(handler, level+1,node->children[i], getChildCube(cube,i));
			}
		}
	}
}

void Octree::iterate(IteratorHandler * handler) {
	BoundingCube cube(glm::vec3(getMinX(),getMinY(),getMinZ()),getLength());
	iterateAux(handler, 0, root, cube);
}



void saveAux(std::ofstream * myfile, OctreeNode * node) {
	if(node!= NULL) {
		Vertex vertex = node->vertex;
		*myfile << "\n{";
		*myfile << "\"v\":" << node->vertex.toString() << ",";
		*myfile << "\"s\":" << (int) node->solid << ",";
		*myfile << "\"h\":" << node->height;
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
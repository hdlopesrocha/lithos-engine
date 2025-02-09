#include <bitset>
#include "tools.hpp"
#include <glm/gtx/norm.hpp> 
#include <filesystem>
#include <iostream>
#include <fstream>

OctreeNode * load(int i, std::vector<OctreeNodeSerialized> * nodes) {
	OctreeNodeSerialized serialized = nodes->at(i);
	Vertex vertex(serialized.position, serialized.normal, glm::vec2(0), serialized.texIndex);

	OctreeNode * node = new OctreeNode(vertex);
	node->mask = serialized.mask;
	node->solid = serialized.solid;

	for(int j=0 ; j <8 ; ++j){
		int index = serialized.children[j];
		if(index != 0) {
			//std::cout << "node[" << std::to_string(index) << "] = " << node->vertex.toString() << std::endl;
			node->setChild(j , load(index, nodes));
		}
	}

	return node;
}


OctreeLoader::OctreeLoader(Octree * tree, std::string filename) {
	this->tree = tree;

	this->file = std::ifstream("data/"+filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return;
    }

	size_t size;
	file.read(reinterpret_cast<char*>(&size), sizeof(size_t) );
	//std::cout << "Loading " << std::to_string(size) << " nodes" << std::endl;

	std::vector<OctreeNodeSerialized> nodes(size);
   	file.read(reinterpret_cast<char*>(nodes.data()), size * sizeof(OctreeNodeSerialized));

	tree->root = load(0,&nodes);
    file.close();
}



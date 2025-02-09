#include <bitset>
#include "tools.hpp"
#include <glm/gtx/norm.hpp> 
#include <filesystem>
#include <iostream>
#include <fstream>

void ensureFolderExists(const std::string& folder) {
    if (!std::filesystem::exists(folder)) {
        std::filesystem::create_directories(folder);
    }
}

uint save(OctreeNode * node, std::vector<OctreeNodeSerialized*> * nodes) {
	if(node!=NULL) {
		OctreeNodeSerialized * n = new OctreeNodeSerialized();
		n->position = node->vertex.position;
		n->normal = node->vertex.normal;
		n->texIndex = node->vertex.texIndex;
		n->mask = node->mask;
		n->solid = node->solid;
		uint index = nodes->size(); 
		nodes->push_back(n);

		for(int i=0; i < 8; ++i) {
			n->children[i] = save(node->children[i], nodes);
		}
		return index;
	}
	return 0;
}


OctreeSaver::OctreeSaver(Octree * tree, std::string filename) {
	this->tree = tree;
	ensureFolderExists("data");
    std::vector<OctreeNodeSerialized*> nodes;

	this->file = std::ofstream("data/"+filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }
	save(tree->root, &nodes);

	OctreeSerialized  octreeSerialized;
	octreeSerialized.min = tree->getMin();
	octreeSerialized.length = tree->getLength();
	octreeSerialized.minSize = tree->minSize;

	file.write(reinterpret_cast<const char*>(&octreeSerialized), sizeof(OctreeSerialized));

	size_t size = nodes.size();
	std::cout << "Saving " << std::to_string(size) << " nodes" << std::endl;
	std::cout << std::to_string(sizeof(OctreeNodeSerialized)) << " bytes/node" << std::endl;

	file.write(reinterpret_cast<const char*>(&size), sizeof(size_t) );

	for(int i=0; i < nodes.size(); ++i) {
		OctreeNodeSerialized * n = nodes[i];
		file.write(reinterpret_cast<const char*>(n), sizeof(OctreeNodeSerialized) );
	}
	file.close();
	nodes.clear();
}


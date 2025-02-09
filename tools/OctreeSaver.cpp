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
		n->solid = n->solid;
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

	this->file = std::ofstream("data/"+filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }
	save(tree->root, &this->nodes);

	size_t size = nodes.size();
	//std::cout << "Saving " << std::to_string(size) << " nodes" << std::endl;

	file.write(reinterpret_cast<const char*>(&size), sizeof(size_t) );

	for(int i=0; i < this->nodes.size(); ++i) {
		OctreeNodeSerialized * n = this->nodes[i];
		file.write(reinterpret_cast<const char*>(n), sizeof(OctreeNodeSerialized) );
	}
	file.close();
}


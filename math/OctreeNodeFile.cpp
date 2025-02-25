#include <bitset>
#include "math.hpp"
#include <glm/gtx/norm.hpp> 



OctreeNodeFile::OctreeNodeFile(OctreeNode * node, std::string filename) {
	this->node = node;
	this->filename = filename;
}

OctreeNode * loadRecursive2(OctreeNode * node, int i, std::vector<OctreeNodeSerialized> * nodes) {
	OctreeNodeSerialized serialized = nodes->at(i);
	Vertex vertex(serialized.position, serialized.normal, glm::vec2(0), serialized.brushIndex);
	if(node == NULL) {
		node = new OctreeNode(vertex);
		node->mask = serialized.mask;
		node->solid = serialized.solid;
	}

	for(int j=0 ; j <8 ; ++j){
		int index = serialized.children[j];
		if(index != 0) {
			node->setChild(j , loadRecursive2(NULL, index, nodes));
		}
	}

	return node;
}


void OctreeNodeFile::load() {
	std::ifstream file = std::ifstream(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return;
    }

    std::stringstream decompressed = gzipDecompressFromIfstream(file);

	size_t size;
	decompressed.read(reinterpret_cast<char*>(&size), sizeof(size_t) );

	std::vector<OctreeNodeSerialized> nodes;
	nodes.resize(size);

   	decompressed.read(reinterpret_cast<char*>(nodes.data()), size * sizeof(OctreeNodeSerialized));
	loadRecursive2(node, 0, &nodes);
    file.close();
	nodes.clear();
}


uint saveRecursive2(OctreeNode * node, std::vector<OctreeNodeSerialized*> * nodes) {
	if(node!=NULL) {
		OctreeNodeSerialized * n = new OctreeNodeSerialized();
		n->position = node->vertex.position;
		n->normal = node->vertex.normal;
		n->brushIndex = node->vertex.brushIndex;
		n->mask = node->mask;
		n->solid = node->solid;
		uint index = nodes->size(); 
		nodes->push_back(n);

		for(int i=0; i < 8; ++i) {
            n->children[i] = saveRecursive2(node->children[i], nodes);
		}
		return index;
	}
	return 0;
}

void OctreeNodeFile::save(){
    std::vector<OctreeNodeSerialized*> nodes;

	std::ofstream file = std::ofstream(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }

	saveRecursive2(node, &nodes);

	size_t size = nodes.size();
	//std::cout << "Saving " << std::to_string(size) << " nodes" << std::endl;
	//std::cout << std::to_string(sizeof(OctreeNodeSerialized)) << " bytes/node" << std::endl;
    std::ostringstream decompressed;
	decompressed.write(reinterpret_cast<const char*>(&size), sizeof(size_t) );
	for(int i=0; i < nodes.size(); ++i) {
		OctreeNodeSerialized * n = nodes[i];
		decompressed.write(reinterpret_cast<const char*>(n), sizeof(OctreeNodeSerialized) );
	}
	
	std::istringstream inputStream(decompressed.str());
 	gzipCompressToOfstream(inputStream, file);
	file.close();

	for(OctreeNodeSerialized * n : nodes) {
		delete n;
	}
	nodes.clear();
}
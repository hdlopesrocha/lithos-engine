#include <bitset>
#include "math.hpp"
#include <glm/gtx/norm.hpp> 



OctreeFile::OctreeFile(Octree * tree, std::string filename, int chunkHeight) {
	this->tree = tree;
	this->chunkHeight = chunkHeight;
	this->filename = filename;
}

OctreeNode * loadRecursive(int i, std::vector<OctreeNodeSerialized> * nodes, int height, std::string filename) {
	OctreeNodeSerialized serialized = nodes->at(i);
	Vertex vertex(serialized.position, serialized.normal, glm::vec2(0), serialized.texIndex);

	OctreeNode * node = new OctreeNode(vertex);
	node->mask = serialized.mask;
	node->solid = serialized.solid;

	
	if(height>0) {
		for(int j=0 ; j <8 ; ++j){
			int index = serialized.children[j];
			if(index != 0) {
				node->setChild(j , loadRecursive(index, nodes, height -1, filename + std::to_string(j)));
			}
		}
	}else {
		OctreeNodeFile * file = new OctreeNodeFile(node, "data/" + filename + ".bin");
		NodeInfo info;
		info.data = file;
		info.type = INFO_TYPE_FILE;	
		info.dirty = true;	
		node->info.push_back(info);
	}
	return node;
}


void OctreeFile::load() {
	std::ifstream file = std::ifstream("data/"+filename+".bin", std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return;
    }

	OctreeSerialized octreeSerialized;
	file.read(reinterpret_cast<char*>(&octreeSerialized), sizeof(OctreeSerialized) );

	size_t size;
	file.read(reinterpret_cast<char*>(&size), sizeof(size_t) );
	std::cout << "Loading " << std::to_string(size) << " nodes" << std::endl;
	std::cout << "Octree: l=" << std::to_string(octreeSerialized.length) << ", mS=" << std::to_string(octreeSerialized.minSize) << ", min={" <<  std::to_string(octreeSerialized.min.x) << "," << std::to_string(octreeSerialized.min.y) << "," << std::to_string(octreeSerialized.min.z) <<"}" << std::endl;

	std::vector<OctreeNodeSerialized> nodes(size);
   	file.read(reinterpret_cast<char*>(nodes.data()), size * sizeof(OctreeNodeSerialized));

	tree->minSize = octreeSerialized.minSize;
	tree->setMin(octreeSerialized.min);
	tree->setLength(octreeSerialized.length);

	int height = tree->getHeight(*tree);
	tree->root = loadRecursive(0,&nodes, height - chunkHeight, filename + "-");

    file.close();
	nodes.clear();
}



uint saveRecursive(OctreeNode * node, std::vector<OctreeNodeSerialized*> * nodes, int height, std::string filename) {
	if(node!=NULL) {
		OctreeNodeSerialized * n = new OctreeNodeSerialized();
		n->position = node->vertex.position;
		n->normal = node->vertex.normal;
		n->texIndex = node->vertex.texIndex;
		n->mask = node->mask;
		n->solid = node->solid;
		uint index = nodes->size(); 
		nodes->push_back(n);
		if(height>0) {
			for(int i=0; i < 8; ++i) {
				n->children[i] = saveRecursive(node->children[i], nodes, height -1, filename + std::to_string(i));
			}
		} else {
			OctreeNodeFile file(node, "data/" + filename + ".bin");
			file.save();
		}
		return index;
	}
	return 0;
}

void OctreeFile::save(){
	ensureFolderExists("data");
    std::vector<OctreeNodeSerialized*> nodes;

	std::ofstream file = std::ofstream("data/"+filename+".bin", std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }

	int height = tree->getHeight(*tree);
	saveRecursive(tree->root, &nodes, height - chunkHeight, filename + "-");

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
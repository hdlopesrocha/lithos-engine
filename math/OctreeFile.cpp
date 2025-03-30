#include <bitset>
#include "math.hpp"



OctreeFile::OctreeFile(Octree * tree, std::string filename, int chunkHeight) {
	this->tree = tree;
	this->chunkHeight = chunkHeight;
	this->filename = filename;
}

std::string getChunkName(BoundingCube cube) {
	glm::vec3 p = cube.getMin();
	return std::to_string(cube.getLengthX()) + "_" + std::to_string(p.x) + "_" +  std::to_string(p.y) + "_" + std::to_string(p.z);
}

OctreeNode * loadRecursive(int i, std::vector<OctreeNodeSerialized> * nodes, int height, std::string filename, BoundingCube cube, std::string baseFolder) {
	OctreeNodeSerialized serialized = nodes->at(i);
	Vertex vertex(serialized.position, serialized.normal, glm::vec2(0), serialized.brushIndex);

	OctreeNode * node = new OctreeNode(vertex);
	node->mask = serialized.mask;
	node->solid = serialized.solid;

	
	if(height>0) {
		for(int j=0 ; j <8 ; ++j){
			int index = serialized.children[j];
			if(index != 0) {
				BoundingCube c = Octree::getChildCube(cube, j);
				node->setChild(j , loadRecursive(index, nodes, height -1, filename, c,baseFolder));
			}
		}
	}else {
		std::string chunkName = getChunkName(cube);
		OctreeNodeFile * file = new OctreeNodeFile(node, baseFolder + "/" + filename+ "_" + chunkName + ".bin");
		NodeInfo info(INFO_TYPE_FILE, file, NULL, true);
		//node->info.push_back(info);
		file->load(baseFolder);
		delete file;
	}
	return node;
}


void OctreeFile::load(std::string baseFolder) {
	std::string filePath = baseFolder + "/" + filename+".bin";
	std::ifstream file = std::ifstream(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for reading: " << filePath << std::endl;
        return;
    }

    std::stringstream decompressed = gzipDecompressFromIfstream(file);


	OctreeSerialized octreeSerialized;
	decompressed.read(reinterpret_cast<char*>(&octreeSerialized), sizeof(OctreeSerialized) );

	size_t size;
	decompressed.read(reinterpret_cast<char*>(&size), sizeof(size_t) );
	//std::cout << "Loading " << std::to_string(size) << " nodes" << std::endl;
	//std::cout << "Octree: l=" << std::to_string(octreeSerialized.length) << ", mS=" << std::to_string(octreeSerialized.minSize) << ", min={" <<  std::to_string(octreeSerialized.min.x) << "," << std::to_string(octreeSerialized.min.y) << "," << std::to_string(octreeSerialized.min.z) <<"}" << std::endl;

	std::vector<OctreeNodeSerialized> nodes;
	nodes.resize(size);
   	decompressed.read(reinterpret_cast<char*>(nodes.data()), size * sizeof(OctreeNodeSerialized));

	tree->minSize = octreeSerialized.minSize;
	tree->setMin(octreeSerialized.min);
	tree->setLength(octreeSerialized.length);

	int height = tree->getHeight(*tree);
	tree->root = loadRecursive(0,&nodes, height - chunkHeight, filename, *tree, baseFolder);

    file.close();
	nodes.clear();

	std::cout << "OctreeFile::load('" << filePath <<"') Ok!" << std::endl;
}


uint saveRecursive(OctreeNode * node, std::vector<OctreeNodeSerialized*> * nodes, int height, std::string filename, BoundingCube cube, std::string baseFolder) {
	if(node!=NULL) {
		OctreeNodeSerialized * n = new OctreeNodeSerialized();
		n->position = node->vertex.position;
		n->normal = node->vertex.normal;
		n->brushIndex = node->vertex.brushIndex;
		n->mask = node->mask;
		n->solid = node->solid;
		uint index = nodes->size(); 
		nodes->push_back(n);

		if(height>0) {
			for(int i=0; i < 8; ++i) {
				BoundingCube c = Octree::getChildCube(cube, i);
				n->children[i] = saveRecursive(node->children[i], nodes, height -1, filename, c, baseFolder);
			}
		} else {
			std::string chunkName = getChunkName(cube);
			OctreeNodeFile file(node, baseFolder + "/" + filename + "_" + chunkName + ".bin");
			file.save(baseFolder);
		}
		return index;
	}
	return 0;
}

void OctreeFile::save(std::string baseFolder){
	ensureFolderExists(baseFolder);
    std::vector<OctreeNodeSerialized*> nodes;
	std::string filePath = baseFolder + "/" + filename+".bin";
	std::ofstream file = std::ofstream(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for writing: " << filePath << std::endl;
        return;
    }

	int height = tree->getHeight(*tree);
	saveRecursive(tree->root, &nodes, height - chunkHeight, filename, *tree, baseFolder);

	OctreeSerialized  octreeSerialized;
	octreeSerialized.min = tree->getMin();
	octreeSerialized.length = tree->getLengthX();
	octreeSerialized.minSize = tree->minSize;

    std::ostringstream decompressed;
	decompressed.write(reinterpret_cast<const char*>(&octreeSerialized), sizeof(OctreeSerialized));

	size_t size = nodes.size();
	//std::cout << "Saving " << std::to_string(size) << " nodes" << std::endl;
	//std::cout << std::to_string(sizeof(OctreeNodeSerialized)) << " bytes/node" << std::endl;

	decompressed.write(reinterpret_cast<const char*>(&size), sizeof(size_t) );
	for(OctreeNodeSerialized *n : nodes) {
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

AbstractBoundingBox& OctreeFile::getBox(){
	return *this->tree;
}

#include "space.hpp"



OctreeFile::OctreeFile(Octree * tree, std::string filename) {
	this->tree = tree;
	this->filename = filename;
}

std::string getChunkName(BoundingCube cube) {
	glm::vec3 p = cube.getMin();
	return std::to_string(cube.getLengthX()) + "_" + std::to_string(p.x) + "_" +  std::to_string(p.y) + "_" + std::to_string(p.z);
}

OctreeNode * OctreeFile::loadRecursive(int i, std::vector<OctreeNodeSerialized> * nodes, float chunkSize, std::string filename, BoundingCube cube, std::string baseFolder) {
	OctreeNodeSerialized serialized = nodes->at(i);
	glm::vec3 position = SDF::getAveragePosition(serialized.sdf, cube);
	glm::vec3 normal = SDF::getNormalFromPosition(serialized.sdf, cube, position);
	Vertex vertex(position, normal, glm::vec2(0), serialized.brushIndex);

	OctreeNode * node = tree->allocator.allocateOctreeNode(cube)->init(vertex);
	node->setSDF(serialized.sdf);
	node->bits = serialized.bits;
	if(node->isChunk()){
		node->setDirty(true);
	}
	bool isLeaf = true;
	for(int j=0; j < 8; ++j) {
		if(serialized.children[j] != 0) {
			isLeaf = false;
			break;
		}
	}
	ChildBlock * block = isLeaf ? NULL : node->createBlock(&tree->allocator);
	if(cube.getLengthX() > chunkSize) {
		for(int j=0 ; j <8 ; ++j){
			int index = serialized.children[j];
			if(index != 0) {
				BoundingCube c = cube.getChild(j);
				node->setChildNode(j , loadRecursive(index, nodes, chunkSize, filename, c,baseFolder), &tree->allocator, block);
			}
		}
	} else {
		std::string chunkName = getChunkName(cube);
		OctreeNodeFile * file = new OctreeNodeFile(tree, node, baseFolder + "/" + filename+ "_" + chunkName + ".bin");
		//NodeInfo info(INFO_TYPE_FILE, file, NULL, true);
		//node->info.push_back(info);
		file->load(baseFolder, cube);
		delete file;
	}
	return node;
}


void OctreeFile::load(std::string baseFolder, float chunkSize) {
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

	tree->setMin(octreeSerialized.min);
	tree->setLength(octreeSerialized.length);
	tree->chunkSize = octreeSerialized.chunkSize;
	tree->root = loadRecursive(0,&nodes, chunkSize, filename, *tree, baseFolder);

    file.close();
	nodes.clear();

	std::cout << "OctreeFile::load('" << filePath <<"') Ok!" << std::endl;
}


uint OctreeFile::saveRecursive(OctreeNode * node, std::vector<OctreeNodeSerialized> * nodes, float chunkSize, std::string filename, BoundingCube cube, std::string baseFolder) {
	if(node!=NULL) {
		OctreeNodeSerialized n = OctreeNodeSerialized();
		n.brushIndex = node->vertex.brushIndex;
		n.bits = node->bits;
		SDF::copySDF(node->sdf, n.sdf);

		uint index = nodes->size(); 
		nodes->push_back(n);

		if(cube.getLengthX() > chunkSize) {
			ChildBlock * block = node->getBlock(&tree->allocator);
			for(int i=0; i < 8; ++i) {
				BoundingCube c = cube.getChild(i);
				(*nodes)[index].children[i] = saveRecursive(node->getChildNode(i, &tree->allocator, block), nodes, chunkSize, filename, c, baseFolder);
			}
		} else {
			std::string chunkName = getChunkName(cube);
			OctreeNodeFile file(tree, node, baseFolder + "/" + filename + "_" + chunkName + ".bin");
			file.save(baseFolder);
		}
		return index;
	}
	return 0;
}

void OctreeFile::save(std::string baseFolder, float chunkSize){
	ensureFolderExists(baseFolder);
    std::vector<OctreeNodeSerialized> nodes;
	std::string filePath = baseFolder + "/" + filename+".bin";
	std::ofstream file = std::ofstream(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for writing: " << filePath << std::endl;
        return;
    }

	saveRecursive(tree->root, &nodes, chunkSize, filename, *tree, baseFolder);

	OctreeSerialized  octreeSerialized;
	octreeSerialized.min = tree->getMin();
	octreeSerialized.length = tree->getLengthX();
	octreeSerialized.chunkSize = tree->chunkSize;

    std::ostringstream decompressed;
	decompressed.write(reinterpret_cast<const char*>(&octreeSerialized), sizeof(OctreeSerialized));

	size_t size = nodes.size();
	//std::cout << "Saving " << std::to_string(size) << " nodes" << std::endl;
	//std::cout << std::to_string(sizeof(OctreeNodeSerialized)) << " bytes/node" << std::endl;

	decompressed.write(reinterpret_cast<const char*>(&size), sizeof(size_t) );
	decompressed.write(reinterpret_cast<const char*>(nodes.data()), nodes.size() * sizeof(OctreeNodeSerialized) );
	
	std::istringstream inputStream(decompressed.str());
 	gzipCompressToOfstream(inputStream, file);
	file.close();

	nodes.clear();

	std::cout << "OctreeFile::save('" << filePath <<"') Ok!" << std::endl;

}

AbstractBoundingBox& OctreeFile::getBox(){
	return *this->tree;
}

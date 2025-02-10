#include <bitset>
#include "../math/math.hpp"
#include "gl.hpp"
#include <glm/gtx/norm.hpp> 

OctreeProcessor::OctreeProcessor(Octree * tree, int * triangles,  int drawableType, int geometryLevel, float simplificationAngle, float simplificationDistance, bool simplificationTexturing) {
	this->tree = tree;
	this->simplificationAngle = simplificationAngle;
	this->simplificationDistance = simplificationDistance;
	this->simplificationTexturing = simplificationTexturing;
	this->drawableType = drawableType;
	this->geometryLevel = geometryLevel;
	this->triangles = triangles;
}

void OctreeProcessor::update(glm::mat4 m) {
	frustum = Frustum(m);
}

OctreeNode * OctreeProcessor::getChild(OctreeNode * node, int index){
	return node->children[index];
}
static int simplficationId = 0;

void markNeighborsAsDirty(Octree * tree, BoundingCube cube, int level, int drawableType) {
	for(int i=1; i < 7 ; ++i) {
		glm::vec3 p = cube.getCenter() + cube.getLength()* Octree::getShift(i);
		OctreeNode * n = tree->getNodeAt(p, level, 0);
		if(n!=NULL) {
			for(int j=0; j < n->info.size(); ++j){
				NodeInfo * info = &n->info[j];
				if(info->type == drawableType) {
					DrawableGeometry * geo = (DrawableGeometry*) info->data;
					geo->dirty = true;
				}
			}
		}
	}
}

bool shouldRemove(const NodeInfo& item) {
    return item.type == INFO_TYPE_REMOVE;
}

void removeDirtyNodeInfo(OctreeNode * node, int drawableType) {
	for(int i=0; i < node->info.size(); ++i){
		NodeInfo * info = &node->info[i];
		if(info->type == drawableType) {
			DrawableGeometry * geo = (DrawableGeometry*) info->data;
			if(geo->dirty) {
				std::cout << "Marked dirty" << std::endl;
				info->type = INFO_TYPE_REMOVE;
			}
		}
	}
    node->info.erase(std::remove_if(node->info.begin(), node->info.end(), shouldRemove), node->info.end());
}

void * OctreeProcessor::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
	bool canGenerate = true;
	removeDirtyNodeInfo(node, drawableType);
	
	for(int i=0; i < node->info.size(); ++i){
		NodeInfo * info = &node->info[i];
		if(info->type == drawableType) {
			canGenerate = false;
		}
		if(info->type == INFO_TYPE_FILE) {
			OctreeNodeFile *f = (OctreeNodeFile*) info->data;
			f->load();
			markNeighborsAsDirty(tree, cube, level, drawableType);
		}
	}

	if(tree->getHeight(cube)==geometryLevel){
		if(canGenerate && loaded == 0){
			std::cout << "Generate " << node->vertex.toString() << std::endl;

			++simplficationId;
			// Simplify
			Simplifier simplifier(tree, simplificationAngle, simplificationDistance, simplificationTexturing, simplficationId); 
			simplifier.iterate(level, node, cube, &cube);
			
			// Tesselate
			Geometry * chunk = new Geometry();
			Tesselator tesselator(tree, triangles, chunk, simplficationId);
			tesselator.iterate(level, node, cube, NULL);
			// Send to GPU
			NodeInfo info;
			info.data = new DrawableGeometry(chunk);
			info.type = drawableType;
			node->info.push_back(info);
			
			delete chunk;
			++loaded;
		}
	
		return node;
	}
	return NULL; 			 			
}

void OctreeProcessor::after(int level, OctreeNode * node, BoundingCube cube, void * context) {			
	return;
}

bool OctreeProcessor::test(int level, OctreeNode * node, BoundingCube cube, void * context) {	
	BoundingBox box = BoundingBox(cube.getMin()-tree->minSize, cube.getMax());
	return frustum.isBoxVisible(box) && context == NULL;
}


void OctreeProcessor::getOrder(OctreeNode * node, BoundingCube cube, int * order){
	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}



#include <bitset>
#include "../math/math.hpp"
#include "gl.hpp"
#include <glm/gtx/norm.hpp> 
#include <algorithm>
#include <random>

// Random number generator
std::random_device rd;
std::mt19937 g(rd());  // Mersenne Twister engine

OctreeProcessor::OctreeProcessor(Octree * tree, int * instancesCount,  int drawableType, int geometryLevel, float simplificationAngle, float simplificationDistance, bool simplificationTexturing, bool createInstances, int simplification) {
	this->tree = tree;
	this->simplification= simplification;
	this->simplificationAngle = simplificationAngle;
	this->simplificationDistance = simplificationDistance;
	this->simplificationTexturing = simplificationTexturing;
	this->drawableType = drawableType;
	this->geometryLevel = geometryLevel;
	this->instancesCount = instancesCount;
	this->createInstances = createInstances;
}


void markNeighborsAsDirty(Octree * tree, BoundingCube cube, int level, int drawableType) {
	for(int i=1; i < 7 ; ++i) {
		glm::vec3 p = cube.getCenter() - cube.getLength()* Octree::getShift(i);
		OctreeNode * n = tree->getNodeAt(p, level, 0);
		if(n!=NULL) {
			for(int j=0; j < n->info.size(); ++j){
				NodeInfo * info = &n->info[j];
				if(info->type == drawableType) {
					info->dirty = true;
				}
			}
		}
	}
}

void * OctreeProcessor::before(int level, OctreeNode * node, BoundingCube cube, void * context) {		
	if(loadCount > 0) {
		bool canGenerate = true;
		for(int i=0; i < node->info.size(); ++i){
			NodeInfo * info = &node->info[i];
			if(info->type == INFO_TYPE_FILE && info->dirty) {
				OctreeNodeFile *f = (OctreeNodeFile*) info->data;
				f->load();
				markNeighborsAsDirty(tree, cube, level, drawableType);
				info->dirty = false;
			}
			if(info->type == drawableType && !info->dirty) {
				canGenerate = false;
			}
		}
		float height = tree->getHeight(cube);
		int currentLod = height - geometryLevel;


		if(height==geometryLevel){
			if(canGenerate && createInstances) {
				if(drawableType == TYPE_INSTANCE_SOLID_DRAWABLE || drawableType == TYPE_INSTANCE_LIQUID_DRAWABLE) {

					// Simplify
					Simplifier simplifier(tree, simplificationAngle, simplificationDistance, simplificationTexturing, simplification); 
					simplifier.iterate(level, node, cube, &cube);

					// Tesselate
					Geometry * geometry = new Geometry();
					Tesselator tesselator(tree, geometry, simplification);
					tesselator.iterate(level, node, cube, NULL);

					NodeInfo * info = node->getNodeInfo(drawableType);
					if(info == NULL) {
						PreLoadedGeometry * pre = new PreLoadedGeometry();
						pre->center = cube.getCenter();
						pre->instances.push_back(InstanceData(glm::mat4(1.0), 0.0f));
						pre->geometry = geometry;

						node->info.push_back(NodeInfo(drawableType, NULL, pre, false));
						*instancesCount += 1;
						--loadCount;
					}
				}


				// Instances with LOD
				if(drawableType == TYPE_INSTANCE_VEGETATION_DRAWABLE) {
					NodeInfo * info = node->getNodeInfo(drawableType);
					if(info == NULL) {
						PreLoadedGeometry * pre = new PreLoadedGeometry();
						pre->center = cube.getCenter();
						pre->geometry = new Vegetation3d();

						InstanceBuilder instanceBuilder(tree, currentLod, &pre->instances);
						instanceBuilder.iterate(level, node, cube, NULL);

						// Shuffle the vector
						std::shuffle(pre->instances.begin(), pre->instances.end(), g);
						
						node->info.push_back(NodeInfo(drawableType, NULL, pre, false));
						*instancesCount += instanceBuilder.instanceCount;
						--loadCount;
					}
				}

			}
			return node;
		}
	}
	return NULL; 			 			
}

void OctreeProcessor::after(int level, OctreeNode * node, BoundingCube cube, void * context) {			
	return;
}

bool OctreeProcessor::test(int level, OctreeNode * node, BoundingCube cube, void * context) {	
	return loadCount > 0;
}

void OctreeProcessor::getOrder(OctreeNode * node, BoundingCube cube, int * order){
	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}



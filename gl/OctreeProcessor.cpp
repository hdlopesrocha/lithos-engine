#include <bitset>
#include "../math/math.hpp"
#include "gl.hpp"
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


void markNeighborsAsDirty(Octree * tree, const BoundingCube &cube, int level, int drawableType) {
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

void * OctreeProcessor::before(int level, int height, OctreeNode * node, const BoundingCube &cube, void * context) {		
	if(loadCount > 0) {
		bool canGenerate = false;
		NodeInfo * genetator;

		for(NodeInfo &info : node->info) {
			if(info.type == INFO_TYPE_FILE && info.dirty) {
				OctreeNodeFile *f = (OctreeNodeFile*) info.data;
				f->load();
				markNeighborsAsDirty(tree, cube, level, drawableType);
				info.dirty = false;
			}
			if(info.dirty) {
				genetator = &info;
				canGenerate = true;
			}
		}
		if(genetator == NULL) {
			canGenerate = true;
		}

		int currentLod = height - geometryLevel;


		if(currentLod>=0){
		
			if(canGenerate && createInstances) {
				if(genetator) {
					genetator->dirty = false;
				}
			

				if(drawableType == TYPE_INSTANCE_SOLID_DRAWABLE || drawableType == TYPE_INSTANCE_LIQUID_DRAWABLE || drawableType == TYPE_INSTANCE_SHADOW_DRAWABLE) {
					int tempDrawableType = drawableType == TYPE_INSTANCE_SHADOW_DRAWABLE ? TYPE_INSTANCE_SOLID_DRAWABLE : drawableType;
					NodeInfo * info = node->getNodeInfo(tempDrawableType);
					if(info == NULL) {
						// Simplify
						Simplifier simplifier(tree, cube, simplificationAngle, simplificationDistance, simplificationTexturing, simplification); 
						simplifier.iterateFlatOut(level, height, node, cube, NULL);

						// Tesselate
						Geometry * geometry = new Geometry();
						Tesselator tesselator(tree, geometry, simplification);
						tesselator.iterateFlatIn(level, height, node, cube, NULL);

					
						PreLoadedGeometry * pre = new PreLoadedGeometry();
						pre->center = cube.getCenter();
						pre->instances.push_back(InstanceData(glm::mat4(1.0), 0.0f));
						pre->geometry = geometry;

						node->info.push_back(NodeInfo(tempDrawableType, NULL, pre, false));
						*instancesCount += 1;
						--loadCount;
					}
				}


				// Instances with LOD
				if(drawableType == TYPE_INSTANCE_VEGETATION_DRAWABLE || drawableType == TYPE_INSTANCE_SHADOW_DRAWABLE) {
					int tempDrawableType = drawableType == TYPE_INSTANCE_SHADOW_DRAWABLE ? TYPE_INSTANCE_VEGETATION_DRAWABLE : drawableType;

					NodeInfo * info = node->getNodeInfo(tempDrawableType);
					if(info == NULL) {
						PreLoadedGeometry * pre = new PreLoadedGeometry();
						pre->center = cube.getCenter();
						pre->geometry = new Vegetation3d();

						InstanceBuilder instanceBuilder(tree, currentLod, &pre->instances);
						instanceBuilder.iterateFlatIn(level, height, node, cube, NULL);

						// Shuffle the vector
						std::shuffle(pre->instances.begin(), pre->instances.end(), g);
						
						node->info.push_back(NodeInfo(tempDrawableType, NULL, pre, false));
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

void OctreeProcessor::after(int level, int height, OctreeNode * node, const BoundingCube &cube, void * context) {			
	return;
}

bool OctreeProcessor::test(int level, int height, OctreeNode * node, const BoundingCube &cube, void * context) {	
	return loadCount > 0;
}

void OctreeProcessor::getOrder(const BoundingCube &cube, int * order){
	for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}



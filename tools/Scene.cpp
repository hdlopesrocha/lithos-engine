
#include "tools.hpp"

Scene::Scene(Settings * settings) {
	this->settings = settings;
	solidSpace = new Octree(BoundingCube(glm::vec3(0,0,0), 30.0));
	liquidSpace = new Octree(BoundingCube(glm::vec3(0,7,0), 30.0));

	solidTrianglesCount = 0;
	liquidTrianglesCount = 0;

	solidInstancesVisible = 0;
	liquidInstancesVisible = 0;
	vegetationInstancesVisible = 0;

	chunkSize = glm::pow(2, 9);

	solidBuilder = new MeshGeometryBuilder(&solidTrianglesCount, solidSpace, 0.99, 0.1, true);
	liquidBuilder = new MeshGeometryBuilder(&liquidTrianglesCount, liquidSpace, 0.99, 0.1, true);
	vegetationBuilder = new VegetationGeometryBuilder(solidSpace, new VegetationInstanceBuilderHandler(solidSpace, 32, 4));
	debugBuilder = new OctreeGeometryBuilder(new OctreeInstanceBuilderHandler());

	solidRenderer = new OctreeVisibilityChecker(&visibleSolidNodes);
	liquidRenderer = new OctreeVisibilityChecker(&visibleLiquidNodes);
	for(int i = 0 ; i < SHADOW_MATRIX_COUNT ; ++i) {
		shadowRenderer[i]= new OctreeVisibilityChecker(&visibleShadowNodes[i]);
	}

}

bool Scene::loadSpace(Octree* tree, OctreeNodeData& data, std::unordered_map<long, NodeInfo>* infos, GeometryBuilder* builder) {
	if (data.node->dataId == 0) {
		data.node->dataId = ++tree->dataId;
	}

	InstanceGeometry* loadable = builder->build(data);
	if (loadable == NULL) {
		// No geometry to load — erase entry if it exists
		infos->erase(data.node->dataId);
		return false;
	}

	// Try to insert a new NodeInfo with loadable
	auto [it, inserted] = infos->try_emplace(data.node->dataId, loadable);
	if (!inserted) {
		// Already existed — replace existing loadable
		if (it->second.loadable) {
			delete it->second.loadable;
		}
		it->second.loadable = loadable;
	}

	return true;
}



bool Scene::processLiquid(OctreeNodeData &data, Octree * tree) {
	bool result = false;
	if(data.node->isDirty()) {
		data.node->setDirty(false);
		if(loadSpace(tree, data, &liquidInfo, liquidBuilder)) {
			result = true;			
		}
	}
	return result;
}

bool Scene::processSolid(OctreeNodeData &data, Octree * tree) {
	bool result = false;
	if(data.node->isDirty()) {
		data.node->setDirty(false);
		if(loadSpace(tree, data, &solidInfo, solidBuilder)) {
			result = true;			
		}
		if(loadSpace(tree, data, &vegetationInfo, vegetationBuilder)) {
			result = true;			
		}
		if(loadSpace(tree, data, &debugInfo, debugBuilder)) {
			result = true;			
		}
	}
	return result;
}

void Scene::processSpace() {
	// Set load counts per Processor
	solidInstancesVisible = 0;
	liquidInstancesVisible = 0;
	vegetationInstancesVisible = 0;
	int loadCountSolid = 1;
	int loadCountLiquid = 1;

	for(OctreeNodeData &data : visibleSolidNodes){
		if(loadCountSolid > 0) {
			if(processSolid(data, solidSpace)) {
				--loadCountSolid;
			}
		} else {
			break;
		}
	}

	for(OctreeNodeData &data : visibleLiquidNodes){
		if(loadCountLiquid > 0) {
			if(processLiquid(data, liquidSpace)) {
				--loadCountLiquid;
			}
		} else {
			break;
		}
	}

	for(int i =0 ; i < SHADOW_MATRIX_COUNT ; ++i){
		std::vector<OctreeNodeData> &vec = visibleShadowNodes[i];
		for(OctreeNodeData &data : vec) {
			if(loadCountSolid > 0) {
				if(processSolid(data, solidSpace)) {
					--loadCountSolid;
				}
			} else {
				break;
			}
		}
	}
}

void Scene::setVisibility(glm::mat4 viewProjection, std::vector<std::pair<glm::mat4, glm::vec3>> lightProjection ,Camera &camera) {
	setVisibleNodes(solidSpace, viewProjection, camera.position, *solidRenderer);
	setVisibleNodes(liquidSpace, viewProjection, camera.position, *liquidRenderer);

	int i =0;
	for(std::pair<glm::mat4, glm::vec3> pair :  lightProjection){
		setVisibleNodes(solidSpace, pair.first, pair.second, *shadowRenderer[i++]);
	}
}

void Scene::setVisibleNodes(Octree * tree, glm::mat4 viewProjection, glm::vec3 sortPosition, OctreeVisibilityChecker &checker) {
	checker.visibleNodes->clear();
	checker.sortPosition = sortPosition;
	checker.update(viewProjection);
	tree->iterateFlat(checker, chunkSize);	//here we get the visible nodes for that LOD + geometryLEvel
}

DrawableInstanceGeometry* Scene::loadIfNeeded(std::unordered_map<long, NodeInfo>* infos, long index) {
	auto it = infos->find(index);
	if (it == infos->end()) {
		return NULL;
	}
	NodeInfo& ni = it->second;
	if (ni.loadable) {
		if (ni.drawable) {
			delete ni.drawable;
		}
		ni.drawable = new DrawableInstanceGeometry(ni.loadable->geometry, &ni.loadable->instances);
		delete ni.loadable;
		ni.loadable = NULL;
	}
	return ni.drawable;
}


void Scene::draw (uint drawableType, int mode, glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) {
	for(const OctreeNodeData &data : list) {
		OctreeNode * node = data.node;
		
		if(node->dataId){
			if(drawableType == TYPE_INSTANCE_VEGETATION_DRAWABLE) {
				DrawableInstanceGeometry * drawable	= loadIfNeeded(&vegetationInfo, node->dataId);
				if(drawable != NULL) {
					float amount = glm::clamp( 1.0 - glm::length(cameraPosition -  drawable->center)/(float(settings->billboardRange)), 0.0, 1.0);
					if(amount > 0.8){
						amount = 1.0;
					}
					//std::cout << "Scene.draw() " << std::to_string(drawableType) << "|" << std::to_string(amount) << std::endl;
					drawable->draw(mode, amount, &vegetationInstancesVisible);
				}
			}else if(drawableType == TYPE_INSTANCE_SOLID_DRAWABLE) {
				DrawableInstanceGeometry * drawable	= loadIfNeeded(&solidInfo, node->dataId);
				if(drawable != NULL) {
					drawable->draw(mode, 1.0, &solidInstancesVisible);
				}
			}
			else if(drawableType == TYPE_INSTANCE_LIQUID_DRAWABLE) {
				DrawableInstanceGeometry * drawable	= loadIfNeeded(&liquidInfo, node->dataId);
				if(drawable != NULL) {
					drawable->draw(mode, 1.0, &liquidInstancesVisible);
				}
			}
			else if(drawableType == TYPE_INSTANCE_OCTREE_DRAWABLE) {
				DrawableInstanceGeometry * drawable	= loadIfNeeded(&debugInfo, node->dataId);
				if(drawable != NULL) {
					drawable->draw(mode, 1.0, &liquidInstancesVisible);
				}
			}	
		}
	}
}

void Scene::drawVegetation(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) {
	glDisable(GL_CULL_FACE);
	draw(TYPE_INSTANCE_VEGETATION_DRAWABLE, GL_PATCHES, cameraPosition, list);
	glEnable(GL_CULL_FACE);
}

void Scene::draw3dSolid(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) {
	draw(TYPE_INSTANCE_SOLID_DRAWABLE, GL_PATCHES, cameraPosition, list);
}

void Scene::draw3dLiquid(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) {
	draw(TYPE_INSTANCE_LIQUID_DRAWABLE, GL_PATCHES, cameraPosition, list);
}

void Scene::draw3dOctree(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) {
	draw(TYPE_INSTANCE_OCTREE_DRAWABLE, GL_LINES, cameraPosition, list);
}


void Scene::generate(Camera &camera) {
	int sizePerTile = 30;
	int tiles= 256;
	int height = 2048;
	float minSize = 30;

	solidSpace->add(BoxContainmentHandler(BoundingBox(glm::vec3(1500,0,0),glm::vec3(1500+256,256,256)),
		SimpleBrush(4)), DirtyHandler(*this), 2.0);

	solidSpace->add(BoxContainmentHandler(BoundingBox(glm::vec3(2000,0,0),glm::vec3(2000+256,256,256)),
		SimpleBrush(4)), DirtyHandler(*this), minSize);

	solidSpace->add(BoxContainmentHandler(BoundingBox(glm::vec3(2500,0,0),glm::vec3(2500+256,256,256)),
		SimpleBrush(4)), DirtyHandler(*this), minSize*2);

	BoundingBox mapBox = BoundingBox(glm::vec3(-sizePerTile*tiles*0.5,-height*0.5,-sizePerTile*tiles*0.5), glm::vec3(sizePerTile*tiles*0.5,height*0.5,sizePerTile*tiles*0.5));
	camera.position.x = mapBox.getCenter().x;
	camera.position.y = mapBox.getMaxY();
	camera.position.z = mapBox.getCenter().z;

	solidSpace->add(HeightMapContainmentHandler(
		HeightMap(
			CachedHeightMapSurface(
				GradientPerlinSurface(height, 1.0/(256.0f*sizePerTile), -64), 
				mapBox, sizePerTile
			), mapBox, sizePerTile
		), LandBrush()
	), DirtyHandler(*this), minSize);

	solidSpace->del(SphereContainmentHandler(BoundingSphere(glm::vec3(0,768,0),1024), SimpleBrush(14)), DirtyHandler(*this), minSize);

	solidSpace->add(BoxContainmentHandler(BoundingBox(glm::vec3(1500,0,500),glm::vec3(1500+256,256,500+256)),
		SimpleBrush(4)), DirtyHandler(*this), 2.0);

	solidSpace->add(BoxContainmentHandler(BoundingBox(glm::vec3(2000,0,500),glm::vec3(2000+256,256,500+256)),
		SimpleBrush(4)), DirtyHandler(*this), minSize);
	
	solidSpace->add(BoxContainmentHandler(BoundingBox(glm::vec3(2500,0,500),glm::vec3(2500+256,256,500+256)),
		SimpleBrush(4)), DirtyHandler(*this), minSize*2);

	//solidSpace->add(BoxContainmentHandler(BoundingBox(glm::vec3(-20,-5,-20),glm::vec3(20,10,20)),SimpleBrush(8)), 1.0);
	//solidSpace->del(BoxContainmentHandler(BoundingBox(glm::vec3(-17,-4,-17),glm::vec3(17,12,17)),SimpleBrush(6)), 1.0);
	//liquidSpace->del(BoxContainmentHandler(BoundingBox(glm::vec3(-18,-5,-18),glm::vec3(18,12,18)),SimpleBrush(0)), 1.0);

	
	for(int x= -128; x < 128; ++x) {
		solidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(x,512,0),32), SimpleBrush(3)), DirtyHandler(*this), 2.0);
	}
	solidSpace->del(SphereContainmentHandler(BoundingSphere(glm::vec3(0,512,0),128), SimpleBrush(4)), DirtyHandler(*this), 2.0);

	//solidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(-11,61,11),10), SimpleBrush(5)), 1.0);
	//solidSpace->del(SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),10), SimpleBrush(4)), 1.0);
	//solidSpace->del(SphereContainmentHandler(BoundingSphere(glm::vec3(4,54,-4),8), SimpleBrush(1)), 1.0);

	//liquidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),4), SimpleBrush(0)));
	BoundingBox waterBox = mapBox;
	waterBox.setMaxY(0);
	liquidSpace->add(OctreeContainmentHandler(solidSpace, waterBox, WaterBrush(0)), DirtyHandler(*this), minSize);
}

void Scene::import(const std::string &filename, Camera &camera) {
	int sizePerTile = 30;
	int tiles= 1024;
	int height = 2048;
	float minSize = 30;

	BoundingBox mapBox = BoundingBox(glm::vec3(-sizePerTile*tiles*0.5,-height*0.5,-sizePerTile*tiles*0.5), glm::vec3(sizePerTile*tiles*0.5,height*0.5,sizePerTile*tiles*0.5));
	camera.position.x = mapBox.getCenter().x;
	camera.position.y = mapBox.getMaxY();
	camera.position.z = mapBox.getCenter().z;
	solidSpace->add(HeightMapContainmentHandler(
		HeightMap(
			CachedHeightMapSurface(
				HeightMapTif(filename, mapBox, sizePerTile,1.0f, -320.0f), 
				mapBox,sizePerTile
			), 
			mapBox,sizePerTile
		), 
		DerivativeLandBrush()
	), DirtyHandler(*this), minSize);

	BoundingBox waterBox = mapBox;
	waterBox.setMaxY(0);
	
	liquidSpace->add(OctreeContainmentHandler(solidSpace, waterBox, WaterBrush(0)), DirtyHandler(*this), minSize);
}

void Scene::save(std::string folderPath, Camera &camera) {
	OctreeFile saver1(solidSpace, "solid", 9);
	OctreeFile saver2(liquidSpace, "liquid", 9);
	saver1.save(folderPath, 4096);
	saver2.save(folderPath, 4096);
}

void Scene::load(std::string folderPath, Camera &camera) {
	OctreeFile loader1(solidSpace, "solid", 9);
	OctreeFile loader2(liquidSpace, "liquid", 9);
	loader1.load(folderPath, 4096);
	loader2.load(folderPath, 4096);
	//camera.position.x = loader1.getBox().getCenter().x;
	//camera.position.y = loader1.getBox().getMaxY();
	//camera.position.z = loader1.getBox().getCenter().z;
}

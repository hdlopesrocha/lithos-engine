
#include "tools.hpp"

Scene::Scene(Settings * settings) {
	this->settings = settings;
	solidSpace = new Octree(BoundingCube(glm::vec3(0,0,0), 30.0));
	liquidSpace = new Octree(BoundingCube(glm::vec3(0,7,0), 30.0));

	solidTrianglesCount = 0;
	liquidTrianglesCount = 0;

	solidInstancesCount = 0;
	liquidInstancesCount = 0;
	vegetationInstancesCount = 0;

	solidInstancesVisible = 0;
	liquidInstancesVisible = 0;
	vegetationInstancesVisible = 0;

	chunkSize = glm::pow(2, 11);

	solidProcessor = new OctreeProcessor(solidSpace , true, 
		new MeshGeometryBuilder(TYPE_INSTANCE_SOLID_DRAWABLE, &solidInstancesCount, &solidTrianglesCount, solidSpace, 0.95, 0.5, true)
	);
	liquidProcessor = new OctreeProcessor(liquidSpace, true, 
		new MeshGeometryBuilder(TYPE_INSTANCE_LIQUID_DRAWABLE, &liquidInstancesCount, &liquidTrianglesCount, liquidSpace, 0.95, 0.5, true)
	);
	vegetationProcessor = new OctreeProcessor(solidSpace, true, 
		new VegetationGeometryBuilder(TYPE_INSTANCE_VEGETATION_DRAWABLE, &vegetationInstancesCount, solidSpace, 
			new VegetationInstanceBuilderHandler(solidSpace, &vegetationInstancesCount))
	);
	debugProcessor = new OctreeProcessor(solidSpace, true,  
		new OctreeGeometryBuilder(TYPE_INSTANCE_OCTREE_DRAWABLE, &octreeInstancesCount, solidSpace, 
			new OctreeInstanceBuilderHandler(solidSpace, &octreeInstancesCount))
	);

	solidRenderer = new OctreeVisibilityChecker(solidSpace, &visibleSolidNodes);
	liquidRenderer = new OctreeVisibilityChecker(liquidSpace, &visibleLiquidNodes);
	for(int i = 0 ; i < SHADOW_MATRIX_COUNT ; ++i) {
		shadowRenderer[i]= new OctreeVisibilityChecker(solidSpace, &visibleShadowNodes[i]);
	}

}


void Scene::processSpace() {
	// Set load counts per Processor
	solidInstancesVisible = 0;
	liquidInstancesVisible = 0;
	vegetationInstancesVisible = 0;
	solidProcessor->loadCount = 1;
	liquidProcessor->loadCount = 1;
	vegetationProcessor->loadCount = 1;
	debugProcessor->loadCount = 1;

	for(OctreeNodeData &data : visibleSolidNodes){
		if(solidProcessor->loadCount > 0) {
			solidProcessor->before(data);
		} else {
			break;
		}
	}

	for(OctreeNodeData &data : visibleSolidNodes){
		if(vegetationProcessor->loadCount > 0) {
			vegetationProcessor->before(data);
		} else {
			break;
		}
	}

	for(OctreeNodeData &data : visibleLiquidNodes){
		if(liquidProcessor->loadCount > 0) {
			liquidProcessor->before(data);
		} else {
			break;
		}
	}

	for(int i =0 ; i < SHADOW_MATRIX_COUNT ; ++i){
		std::vector<OctreeNodeData> &vec = visibleShadowNodes[i];
		for(OctreeNodeData &data : vec) {
			if(solidProcessor->loadCount > 0) {
				solidProcessor->before(data);
			}
			else {
				break;
			}
		}
	}

	for(int i =0 ; i < SHADOW_MATRIX_COUNT ; ++i){
		std::vector<OctreeNodeData> &vec = visibleShadowNodes[i];
		for(OctreeNodeData &data : vec) {
			if(vegetationProcessor->loadCount > 0) {
				vegetationProcessor->before(data);
			}
			else {
				break;
			}
		}
	}

	for(OctreeNodeData &data : visibleSolidNodes){
		if(debugProcessor->loadCount > 0) {
			debugProcessor->before(data);
		} else {
			break;
		}
	}
}

void Scene::setVisibility(glm::mat4 viewProjection, std::vector<std::pair<glm::mat4, glm::vec3>> lightProjection ,Camera &camera) {
	setVisibleNodes(viewProjection, camera.position, *solidRenderer);
	setVisibleNodes(viewProjection, camera.position, *liquidRenderer);

	int i =0;
	for(std::pair<glm::mat4, glm::vec3> pair :  lightProjection){
		setVisibleNodes(pair.first, pair.second, *shadowRenderer[i++]);
	}
}

void Scene::setVisibleNodes(glm::mat4 viewProjection, glm::vec3 sortPosition, OctreeVisibilityChecker &checker) {
	checker.visibleNodes->clear();
	checker.sortPosition = sortPosition;
	
	checker.update(viewProjection);
	checker.tree->iterateFlat(checker, chunkSize);	//here we get the visible nodes for that LOD + geometryLEvel
}


void Scene::draw (uint drawableType, int mode, glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) {
	//std::cout << "Scene.draw() " << std::to_string(drawableType) << "|" << std::to_string(list.size()) << std::endl;
	for(const OctreeNodeData &data : list) {
		OctreeNode * node = data.node;
		NodeInfo * info = node->getNodeInfo(drawableType);
		
		if(info != NULL){
			if(info->temp != NULL) {
				InstanceGeometry * pre = (InstanceGeometry *) info->temp;
				info->data = new DrawableInstanceGeometry(drawableType, pre->geometry, &pre->instances);
				info->temp = NULL;
				if(!pre->geometry->reusable){
					delete pre->geometry;
				} 
				delete pre;
			}
			
			DrawableInstanceGeometry * drawable	= (DrawableInstanceGeometry*) info->data;
		
			if(drawableType == TYPE_INSTANCE_VEGETATION_DRAWABLE) {
				float amount = glm::clamp( 1.0 - glm::length(cameraPosition -  drawable->center)/(float(settings->billboardRange)), 0.0, 1.0);
				if(amount > 0.8){
					amount = 1.0;
				}
				drawable->draw(mode, amount, &vegetationInstancesVisible);
			}else if(drawableType == TYPE_INSTANCE_SOLID_DRAWABLE) {
				drawable->draw(mode, 1.0, &solidInstancesVisible);
				solidInstancesVisible += drawable->instancesCount;
			}
			else if(drawableType == TYPE_INSTANCE_LIQUID_DRAWABLE) {
				drawable->draw(mode, 1.0, &liquidInstancesVisible);
			}
			else if(drawableType == TYPE_INSTANCE_OCTREE_DRAWABLE) {
				drawable->draw(mode, 1.0, &solidInstancesVisible);
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
	), minSize);


	solidSpace->del(SphereContainmentHandler(BoundingSphere(glm::vec3(0,768,0),1024), SimpleBrush(14)), minSize);


	BoundingBox waterBox = mapBox;
	waterBox.setMaxY(0);
	
	BoundingBox testBox =BoundingBox(glm::vec3(0,-200,0),glm::vec3(100,100,100));
	liquidSpace->add(BoxContainmentHandler(testBox,SimpleBrush(0)), 1.0);

	//liquidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),4), SimpleBrush(0)));
	liquidSpace->add(OctreeContainmentHandler(solidSpace, waterBox, WaterBrush(0)), minSize);

	//solidSpace->add(BoxContainmentHandler(BoundingBox(glm::vec3(-20,-5,-20),glm::vec3(20,10,20)),SimpleBrush(8)), 1.0);
	//solidSpace->del(BoxContainmentHandler(BoundingBox(glm::vec3(-17,-4,-17),glm::vec3(17,12,17)),SimpleBrush(6)), 1.0);
	//liquidSpace->del(BoxContainmentHandler(BoundingBox(glm::vec3(-18,-5,-18),glm::vec3(18,12,18)),SimpleBrush(0)), 1.0);

	//solidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(0,50,0),20), SimpleBrush(4)), 1.0);
	//solidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(-11,61,11),10), SimpleBrush(5)), 1.0);
	//solidSpace->del(SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),10), SimpleBrush(4)), 1.0);
	//solidSpace->del(SphereContainmentHandler(BoundingSphere(glm::vec3(4,54,-4),8), SimpleBrush(1)), 1.0);


	ContainmentType ct = solidSpace->contains(testBox);
	if(ct == ContainmentType::Contains) {
		std::cout << "\tContains" << std :: endl;
	}
	else if(ct == ContainmentType::Intersects) {
		std::cout << "\tIntersects" << std :: endl;
	}
	else  {
		std::cout << "\tDisjoint" << std :: endl;
	}
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
		LandBrush()
	), minSize);

	BoundingBox waterBox = mapBox;
	waterBox.setMaxY(0);
	
	liquidSpace->add(OctreeContainmentHandler(solidSpace, waterBox, WaterBrush(0)), minSize);

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

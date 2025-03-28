
#include "tools.hpp"

Scene::Scene(Settings * settings) {
	this->settings = settings;
	solidSpace = new Octree(BoundingCube(glm::vec3(0,0,0), 30.0));
	liquidSpace = new Octree(BoundingCube(glm::vec3(0,7,0), 30.0));

	solidInstancesCount = 0;
	liquidInstancesCount = 0;
	vegetationInstancesCount = 0;

	solidInstancesVisible = 0;
	liquidInstancesVisible = 0;
	vegetationInstancesVisible = 0;

	geometryLevel = 6;

	vegetationInstanceHandler= new VegetationInstanceBuilderHandler(solidSpace, &vegetationInstancesCount);


	vegetationBuilder = new VegetationGeometryBuilder(TYPE_INSTANCE_VEGETATION_DRAWABLE, &vegetationInstancesCount, solidSpace, vegetationInstanceHandler);
	meshBuilder = new MeshGeometryBuilder(TYPE_INSTANCE_SOLID_DRAWABLE, &solidInstancesCount, solidSpace, 0.8, 0.5, true);
	liquidMeshBuilder = new MeshGeometryBuilder(TYPE_INSTANCE_LIQUID_DRAWABLE, &liquidInstancesCount, liquidSpace, 0.8, 0.5, true);

	solidProcessor = new OctreeProcessor(solidSpace , true, meshBuilder);
	liquidProcessor = new OctreeProcessor(liquidSpace, true, liquidMeshBuilder);
	vegetationProcessor = new OctreeProcessor(solidSpace, true, vegetationBuilder);

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
	checker.tree->iterateFlat(checker, geometryLevel);	//here we get the visible nodes for that LOD + geometryLEvel
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
				if(drawableType != TYPE_INSTANCE_VEGETATION_DRAWABLE){
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

				//std::cout << "Scene.TYPE_INSTANCE_VEGETATION_DRAWABLE() " << std::to_string(vegetationInstancesVisible) << std::endl;
				drawable->draw(mode, amount, &vegetationInstancesVisible);
			}else if(drawableType == TYPE_INSTANCE_SOLID_DRAWABLE) {
				//std::cout << "Scene.TYPE_INSTANCE_SOLID_DRAWABLE() " << std::to_string(solidInstancesVisible) << std::endl;
				drawable->draw(mode, 1.0, &solidInstancesVisible);
				solidInstancesVisible += drawable->instancesCount;
			}
			else if(drawableType == TYPE_INSTANCE_LIQUID_DRAWABLE) {
				//std::cout << "Scene.TYPE_INSTANCE_LIQUID_DRAWABLE() " << std::to_string(liquidInstancesVisible) << std::endl;
				drawable->draw(mode, 1.0, &liquidInstancesVisible);
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
	if(settings->solidEnabled) {
		draw(TYPE_INSTANCE_SOLID_DRAWABLE, GL_PATCHES, cameraPosition, list);
	}
}

void Scene::draw3dLiquid(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) {
	if(settings->liquidEnabled) {
		draw(TYPE_INSTANCE_LIQUID_DRAWABLE, GL_PATCHES, cameraPosition, list);
	}
}

void Scene::create() {
	int sizePerTile = 30;
	int tiles= 1024;
	int height = 2048;

	BoundingBox mapBox = BoundingBox(glm::vec3(-sizePerTile*tiles*0.5,-height*0.5,-sizePerTile*tiles*0.5), glm::vec3(sizePerTile*tiles*0.5,height*0.5,sizePerTile*tiles*0.5));
	//BoundingBox mapBox = BoundingBox(glm::vec3(0,-512,0), glm::vec3(sizePerTile*1201,4096,sizePerTile*1201));
	
	
	solidSpace->add(HeightMapContainmentHandler(
		HeightMap(
			CachedHeightMapSurface(
				GradientPerlinSurface(height, 1.0/(256.0f*sizePerTile), -64), 
				mapBox, sizePerTile
			), mapBox, sizePerTile
		), LandBrush()
	));


/*
	solidSpace->add(HeightMapContainmentHandler(
		HeightMap(
			CachedHeightMapSurface(
				HeightMapTif("models/output.tif", mapBox, sizePerTile,1.0f, -256.0f-64.0f-32.0f-8.0f), 
				mapBox,sizePerTile
			), 
			mapBox,sizePerTile
		), 
		LandBrush()
	));
*/


	BoundingBox waterBox = mapBox;
	waterBox.setMaxY(0);
	
	//liquidSpace->add(BoxContainmentHandler(BoundingBox(glm::vec3(30,-20,30),glm::vec3(70,20,70)),SimpleBrush(0)));

	//liquidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),4), SimpleBrush(0)));
	liquidSpace->add(OctreeContainmentHandler(solidSpace, waterBox, WaterBrush(0)));
/*
	solidSpace->add(BoxContainmentHandler(BoundingBox(glm::vec3(-20,-5,-20),glm::vec3(20,10,20)),SimpleBrush(8)));
	solidSpace->del(BoxContainmentHandler(BoundingBox(glm::vec3(-17,-4,-17),glm::vec3(17,12,17)),SimpleBrush(6)));
	liquidSpace->del(BoxContainmentHandler(BoundingBox(glm::vec3(-18,-5,-18),glm::vec3(18,12,18)),SimpleBrush(0)));

	solidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(0,50,0),20), SimpleBrush(4)));
	solidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(-11,61,11),10), SimpleBrush(5)));
	solidSpace->del(SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),10), SimpleBrush(4)));
	solidSpace->del(SphereContainmentHandler(BoundingSphere(glm::vec3(4,54,-4),8), SimpleBrush(1)));
*/

}


void Scene::save() {
	OctreeFile saver1(solidSpace, "solid", 9);
	OctreeFile saver2(liquidSpace, "liquid", 9);
	saver1.save();
	saver2.save();
}

void Scene::load() {
	OctreeFile loader1(solidSpace, "solid", 9);
	OctreeFile loader2(liquidSpace, "liquid", 9);
	loader1.load();
	loader2.load();
}

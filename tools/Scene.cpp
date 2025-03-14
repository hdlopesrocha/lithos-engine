
#include "tools.hpp"

void Scene::setup(Settings * settings) {
	this->settings = settings;
}

void Scene::processSpace() {
	// Set load counts per Processor

	solidInstancesVisible = 0;
	liquidInstancesVisible = 0;
	vegetationInstancesVisible = 0;
	solidProcessor.loadCount = 1;
	liquidProcessor.loadCount = 1;
	vegetationProcessor.loadCount = 1;

	for(IteratorData &data : visibleSolidNodes){
		if(solidProcessor.loadCount > 0) {
			solidProcessor.before(data);
		}
		if(vegetationProcessor.loadCount > 0) {
			vegetationProcessor.before(data);
		}
	}

	for(IteratorData &data : visibleLiquidNodes){
		if(liquidProcessor.loadCount > 0) {
			liquidProcessor.before(data);
		}
	}

	for(int i =0 ; i < SHADOW_MATRIX_COUNT ; ++i){
		std::vector<IteratorData> &vec = visibleShadowNodes[i];
		for(IteratorData &data : vec) {
			if(solidProcessor.loadCount > 0) {
				solidProcessor.before(data);
			}
			if(vegetationProcessor.loadCount > 0) {
				vegetationProcessor.before(data);
			}
		}
	}
}

void Scene::setVisibility(glm::mat4 viewProjection, std::vector<std::pair<glm::mat4, glm::vec3>> lightProjection ,Camera &camera) {
	setVisibleNodes(viewProjection, camera.position, solidRenderer);
	setVisibleNodes(viewProjection, camera.position, liquidRenderer);

	int i =0;
	for(std::pair<glm::mat4, glm::vec3> pair :  lightProjection){
		setVisibleNodes(pair.first, pair.second, shadowRenderer[i++]);
	}
}

void Scene::setVisibleNodes(glm::mat4 viewProjection, glm::vec3 sortPosition, OctreeVisibilityChecker &checker) {
	checker.visibleNodes->clear();
	checker.sortPosition = sortPosition;
	
	checker.update(viewProjection);
	checker.tree->iterateFlat(checker, geometryLevel);	//here we get the visible nodes for that LOD + geometryLEvel
}


void Scene::draw (uint drawableType, int mode, glm::vec3 cameraPosition, const std::vector<IteratorData> &list) {
	//std::cout << "Scene.draw() " << std::to_string(drawableType) << "|" << std::to_string(list.size()) << std::endl;
	for(const IteratorData &data : list) {
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

void Scene::drawVegetation(glm::vec3 cameraPosition, const std::vector<IteratorData> &list) {
	glDisable(GL_CULL_FACE);
	draw(TYPE_INSTANCE_VEGETATION_DRAWABLE, GL_PATCHES, cameraPosition, list);
	glEnable(GL_CULL_FACE);
}

void Scene::draw3dSolid(glm::vec3 cameraPosition, const std::vector<IteratorData> &list) {
	draw(TYPE_INSTANCE_SOLID_DRAWABLE, GL_PATCHES, cameraPosition, list);
}

void Scene::draw3dLiquid(glm::vec3 cameraPosition, const std::vector<IteratorData> &list) {
	draw(TYPE_INSTANCE_LIQUID_DRAWABLE, GL_PATCHES, cameraPosition, list);
}

void Scene::create() {

	BoundingBox mapBox(glm::vec3(-200,-60,-200), glm::vec3(200,50,200));
	
	solidSpace->add(HeightMapContainmentHandler(
		HeightMap(
			CachedHeightMapSurface(
				GradientPerlinSurface(100, 1.0f/128.0f, 0), 
				mapBox, solidSpace->minSize
			), mapBox, solidSpace->minSize
		), LandBrush()
	));

	BoundingBox waterBox(glm::vec3(-200,-60,-200), glm::vec3(200,3,200));
	liquidSpace->add(OctreeContainmentHandler(solidSpace, waterBox, WaterBrush(0)));
	liquidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),4), SimpleBrush(0)));

	solidSpace->add(BoxContainmentHandler(BoundingBox(glm::vec3(-20,-5,-20),glm::vec3(20,10,20)),SimpleBrush(8)));
	solidSpace->del(BoxContainmentHandler(BoundingBox(glm::vec3(-17,-4,-17),glm::vec3(17,12,17)),SimpleBrush(8)));
	liquidSpace->del(BoxContainmentHandler(BoundingBox(glm::vec3(-18,-5,-18),glm::vec3(18,12,18)),SimpleBrush(8)));

	solidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(0,50,0),20), SimpleBrush(4)));
	solidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(-11,61,11),10), SimpleBrush(5)));
	solidSpace->del(SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),10), SimpleBrush(4)));
	solidSpace->del(SphereContainmentHandler(BoundingSphere(glm::vec3(4,54,-4),8), SimpleBrush(1)));


}


void Scene::save() {
	OctreeFile saver1(solidSpace, "solid", 6);
	OctreeFile saver2(liquidSpace, "liquid", 6);
	saver1.save();
	saver2.save();
}

void Scene::load() {
	OctreeFile loader1(solidSpace, "solid", 6);
	OctreeFile loader2(liquidSpace, "liquid", 6);
	loader1.load();
	loader2.load();
}

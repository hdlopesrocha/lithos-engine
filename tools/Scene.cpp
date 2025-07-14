
#include "tools.hpp"

Scene::Scene(Settings * settings):
    simplifier(0.99, 0.1, true)

 {
	this->settings = settings;
	solidSpace = new Octree(BoundingCube(glm::vec3(0,0,0), 30.0), glm::pow(2, 9));
	liquidSpace = new Octree(BoundingCube(glm::vec3(0,7,0), 30.0), glm::pow(2, 9));
	brushSpace = new Octree(BoundingCube(glm::vec3(0,0,0), 30.0), glm::pow(2, 9));

	solidTrianglesCount = 0;
	liquidTrianglesCount = 0;
	brushTrianglesCount = 0;

	solidInstancesVisible = 0;
	liquidInstancesVisible = 0;
	vegetationInstancesVisible = 0;
	brushInstancesVisible = 0;
	debugInstancesVisible = 0;

	solidBuilder = new MeshGeometryBuilder(&solidTrianglesCount, solidSpace);
	brushBuilder = new MeshGeometryBuilder(&brushTrianglesCount, brushSpace);
	liquidBuilder = new MeshGeometryBuilder(&liquidTrianglesCount, liquidSpace);
	vegetationBuilder = new VegetationGeometryBuilder(solidSpace, new VegetationInstanceBuilderHandler(solidSpace, 0.1, 4));
	debugBuilder = new OctreeGeometryBuilder(new OctreeInstanceBuilderHandler());

	solidRenderer = new OctreeVisibilityChecker(&visibleSolidNodes);
	brushRenderer = new OctreeVisibilityChecker(&visibleBrushNodes);
	liquidRenderer = new OctreeVisibilityChecker(&visibleLiquidNodes);
	for(int i = 0 ; i < SHADOW_MATRIX_COUNT ; ++i) {
		shadowRenderer[i]= new OctreeVisibilityChecker(&visibleShadowNodes[i]);
	}



	liquidSpaceChangeHandler = new LiquidSpaceChangeHandler(&liquidInfo);
	solidSpaceChangeHandler = new SolidSpaceChangeHandler(&solidInfo, &vegetationInfo, &debugInfo);
	brushSpaceChangeHandler = new BrushSpaceChangeHandler(&brushInfo);
}

template <typename T> bool Scene::loadSpace(Octree* tree, OctreeNodeData& data, std::unordered_map<OctreeNode*, NodeInfo<T>>* infos, GeometryBuilder<T>* builder) {
	auto infoIter = infos->find(data.node);
	if(infoIter != infos->end()) {
        NodeInfo<T>& info = infoIter->second;
		if(info.update) {
			info.update = false;
		} else {
			return false;
		}
	}
	
	InstanceGeometry<T>* loadable = builder->build(data);
	if (loadable == NULL) {
		// No geometry to load — erase entry if it exists
		infos->erase(data.node);
		return false;
	}

	// Try to insert a new NodeInfo with loadable
	auto [it, inserted] = infos->try_emplace(data.node, loadable);
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
	if(data.node->isDirty() && loadSpace(tree, data, &liquidInfo, liquidBuilder)) {
		data.node->setDirty(false);	
		result = true;			
	}
	return result;
}

bool Scene::processSolid(OctreeNodeData &data, Octree * tree) {
	bool result = false;
	if(data.node->isDirty()) { 
		if(loadSpace(tree, data, &solidInfo, solidBuilder)) {
			result = true;		
		}
		if(loadSpace(tree, data, &vegetationInfo, vegetationBuilder)) {
			result = true;			
		}
		if(loadSpace(tree, data, &debugInfo, debugBuilder)) {
			result = true;			
		}
		data.node->setDirty(false);	
	}
	return result;
}

bool Scene::processBrush(OctreeNodeData &data, Octree * tree) {
	bool result = false;
	if(loadSpace(tree, data, &brushInfo, brushBuilder)) {
		result = true;			
	}
	return result;
}

bool Scene::processSpace() {
	// Set load counts per Processor
	solidInstancesVisible = 0;
	liquidInstancesVisible = 0;
	vegetationInstancesVisible = 0;
	brushInstancesVisible = 0;
	debugInstancesVisible = 0;
	int loadCountSolid = 1;
	int loadCountLiquid = 1;

	for(OctreeNodeData &data : visibleSolidNodes) {
		if(loadCountSolid > 0) {
			if(processSolid(data, solidSpace)) {
				--loadCountSolid;
			}
		} else {
			break;
		}
	}

	for(OctreeNodeData &data : visibleBrushNodes) {
		processBrush(data, brushSpace);
	}

	for(OctreeNodeData &data : visibleLiquidNodes) {
		if(loadCountLiquid > 0) {
			if(processLiquid(data, liquidSpace)) {
				--loadCountLiquid;
			}
		} else {
			break;
		}
	}

	for(int i =0 ; i < SHADOW_MATRIX_COUNT ; ++i) {
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

	return loadCountSolid <=0 || loadCountLiquid <=0;
}

void Scene::setVisibility(glm::mat4 viewProjection, std::vector<std::pair<glm::mat4, glm::vec3>> lightProjection ,Camera &camera) {
	setVisibleNodes(solidSpace, viewProjection, camera.position, *solidRenderer);
	setVisibleNodes(liquidSpace, viewProjection, camera.position, *liquidRenderer);
	setVisibleNodes(brushSpace, viewProjection, camera.position, *brushRenderer);

	int i =0;
	for(std::pair<glm::mat4, glm::vec3> pair :  lightProjection){
		setVisibleNodes(solidSpace, pair.first, pair.second, *shadowRenderer[i++]);
	}
}

void Scene::setVisibleNodes(Octree * tree, glm::mat4 viewProjection, glm::vec3 sortPosition, OctreeVisibilityChecker &checker) {
	checker.visibleNodes->clear();
	checker.sortPosition = sortPosition;
	checker.update(viewProjection);
	tree->iterateFlat(checker);	//here we get the visible nodes for that LOD + geometryLevel
}

template <typename T> DrawableInstanceGeometry<T> * Scene::loadIfNeeded(std::unordered_map<OctreeNode*, NodeInfo<T>>* infos, OctreeNode* node, InstanceHandler<T> * handler) {
	auto it = infos->find(node);
	if (it == infos->end()) {
		return NULL;
	}
	NodeInfo<T>& ni = it->second;
	if (ni.loadable) {
		if (ni.drawable) {
			delete ni.drawable;
		}
		ni.drawable = new DrawableInstanceGeometry<T>(ni.loadable->geometry, &ni.loadable->instances, handler);
		delete ni.loadable;
		ni.loadable = NULL;
	}
	return ni.drawable;
}

template <typename T, typename H> void Scene::draw(uint drawableType, int mode, glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list, std::unordered_map<OctreeNode*, NodeInfo<T>> * info, long * count) {
	H handler;
	for(const OctreeNodeData &data : list) {
		OctreeNode * node = data.node;
		DrawableInstanceGeometry<T> * drawable = loadIfNeeded(info, node, &handler);
		
		if(drawableType == TYPE_INSTANCE_AMOUNT_DRAWABLE) {
			if(drawable != NULL) {
				float amount = glm::clamp( 1.0 - glm::length(cameraPosition -  drawable->center)/(float(settings->billboardRange)), 0.0, 1.0);
				if(amount > 0.8){
					amount = 1.0;
				}
				//std::cout << "Scene.draw() " << std::to_string(drawableType) << "|" << std::to_string(amount) << std::endl;
				drawable->draw(mode, amount, count);
			}
		}
		else if(drawableType == TYPE_INSTANCE_FULL_DRAWABLE) {
			if(drawable != NULL) {
				drawable->draw(mode, 1.0, count);
			}
		}	
	}
}

void Scene::drawVegetation(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) {
	glDisable(GL_CULL_FACE);
	draw<InstanceData, InstanceDataHandler>(TYPE_INSTANCE_AMOUNT_DRAWABLE, GL_PATCHES, cameraPosition, list, &vegetationInfo, &vegetationInstancesVisible);
	glEnable(GL_CULL_FACE);
}

void Scene::draw3dSolid(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) {
	draw<InstanceData, InstanceDataHandler>(TYPE_INSTANCE_FULL_DRAWABLE, GL_PATCHES, cameraPosition, list, &solidInfo, &solidInstancesVisible);
}

void Scene::draw3dBrush(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) {
	draw<InstanceData, InstanceDataHandler>(TYPE_INSTANCE_FULL_DRAWABLE, GL_PATCHES, cameraPosition, list, &brushInfo, &brushInstancesVisible);
}

void Scene::draw3dLiquid(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) {
	draw<InstanceData, InstanceDataHandler>(TYPE_INSTANCE_FULL_DRAWABLE, GL_PATCHES, cameraPosition, list, &liquidInfo, &liquidInstancesVisible);
}

void Scene::draw3dOctree(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) {
	draw<DebugInstanceData, DebugInstanceDataHandler>(TYPE_INSTANCE_FULL_DRAWABLE, GL_LINES, cameraPosition, list, &debugInfo, &debugInstancesVisible);
}

void Scene::generate(Camera &camera) {
	int sizePerTile = 30;
	int tiles= 256;
	int height = 2048;
	float minSize = 30;
	Transformation model = Transformation(glm::vec3(1.0f),glm::vec3(0.0f), 0.0f, 0.0f, 0.0f);



	{
		BoundingBox box = BoundingBox(glm::vec3(1500,0,0),glm::vec3(1500+256,256,256));
		BoxDistanceFunction function(box.getCenter(), box.getLength()*0.5f);
		WrappedBox wrappedFunction = WrappedBox(&function, minSize, model);
		solidSpace->add(wrappedFunction, model, SimpleBrush(4), 2.0, simplifier, *solidSpaceChangeHandler);
	}

	{
		BoundingBox box = BoundingBox(glm::vec3(2000,0,0),glm::vec3(2000+256,256,256));
		BoxDistanceFunction function(box.getCenter(), box.getLength()*0.5f);
		WrappedBox wrappedFunction = WrappedBox(&function, minSize, model);
		solidSpace->add(wrappedFunction, model, SimpleBrush(4), minSize, simplifier, *solidSpaceChangeHandler);
	}

	{
		BoundingBox box = BoundingBox(glm::vec3(2500,0,0),glm::vec3(2500+256,256,256));
		BoxDistanceFunction function(box.getCenter(), box.getLength()*0.5f);
		WrappedBox wrappedFunction = WrappedBox(&function, minSize, model);
		solidSpace->add(wrappedFunction, model, SimpleBrush(4), minSize*2, simplifier, *solidSpaceChangeHandler);
	}

	BoundingBox mapBox = BoundingBox(glm::vec3(-sizePerTile*tiles*0.5,-height*0.5,-sizePerTile*tiles*0.5), glm::vec3(sizePerTile*tiles*0.5,height*0.5,sizePerTile*tiles*0.5));
	camera.position.x = mapBox.getCenter().x;
	camera.position.y = mapBox.getMaxY();
	camera.position.z = mapBox.getCenter().z;

	{
		GradientPerlinSurface heightFunction = GradientPerlinSurface(height, 1.0/(256.0f*sizePerTile), -64);
		CachedHeightMapSurface cache = CachedHeightMapSurface(heightFunction, mapBox, sizePerTile);
		HeightMap heightMap = HeightMap(cache, mapBox, sizePerTile);
		HeightMapDistanceFunction function(heightMap);
		WrappedHeightMap wrappedFunction = WrappedHeightMap(&function, minSize, model);
		solidSpace->add(wrappedFunction, model, LandBrush(), minSize, simplifier, *solidSpaceChangeHandler);
	}
	{
		BoundingSphere sphere = BoundingSphere(glm::vec3(0,768,0),1024);
		SphereDistanceFunction function(sphere.center, sphere.radius);
		WrappedSphere wrappedFunction = WrappedSphere(&function, minSize, model);
		solidSpace->del(wrappedFunction, model, SimpleBrush(14), minSize, simplifier, *solidSpaceChangeHandler);
	}
	
 	{
		glm::vec3 a = glm::vec3(0,320, -900);
		glm::vec3 b = glm::vec3(0,-100, -3000);
		float r = 200.0f;
		CapsuleDistanceFunction function(a, b, r);
		WrappedCapsule wrappedFunction = WrappedCapsule(&function, minSize, model);
		solidSpace->add(wrappedFunction, model, SimpleBrush(4), minSize, simplifier, *solidSpaceChangeHandler);
	}
	
	{
		BoundingBox box = BoundingBox(glm::vec3(1500,0,500),glm::vec3(1500+256,256,500+256));
		BoxDistanceFunction function(box.getCenter(), box.getLength()*0.5f);
		WrappedBox wrappedFunction = WrappedBox(&function, minSize, model);
		solidSpace->add(wrappedFunction, model, SimpleBrush(4), 2.0, simplifier, *solidSpaceChangeHandler);
	}

	{
		BoundingBox box = BoundingBox(glm::vec3(2000,0,500),glm::vec3(2000+256,256,500+256));
		BoxDistanceFunction function(box.getCenter(), box.getLength()*0.5f);
		WrappedBox wrappedFunction = WrappedBox(&function, minSize, model);
		solidSpace->add(wrappedFunction, model, SimpleBrush(4), minSize, simplifier, *solidSpaceChangeHandler);
	}

	{
		BoundingBox box = BoundingBox(glm::vec3(2500,0,500),glm::vec3(2500+256,256,500+256));
		BoxDistanceFunction function(box.getCenter(), box.getLength()*0.5f);
		WrappedBox wrappedFunction = WrappedBox(&function, minSize, model);
		solidSpace->add(wrappedFunction, model, SimpleBrush(4), minSize*2, simplifier, *solidSpaceChangeHandler);
	}

	{
		BoundingSphere sphere = BoundingSphere(glm::vec3(0,512,0),128);
		SphereDistanceFunction function(sphere.center, sphere.radius);
		WrappedSphere wrappedFunction = WrappedSphere(&function, minSize, model);
		solidSpace->add(wrappedFunction, model, SimpleBrush(4), 8.0, simplifier, *solidSpaceChangeHandler);
	}

	{
		BoundingBox waterBox = mapBox;
		waterBox.setMaxY(0);
		OctreeDifferenceFunction function(solidSpace, waterBox);
		WrappedOctreeDifference wrappedFunction = WrappedOctreeDifference(&function, minSize, model);
		liquidSpace->add(wrappedFunction, model, WaterBrush(0), minSize, simplifier, *liquidSpaceChangeHandler);
	}
}

void Scene::import(const std::string &filename, Camera &camera) {
	int sizePerTile = 30;
	int tiles= 1024;
	int height = 2048;
	float minSize = 30;
	Transformation model = Transformation(glm::vec3(1.0f),glm::vec3(0.0f), 0.0f, 0.0f, 0.0f);

	BoundingBox mapBox = BoundingBox(glm::vec3(-sizePerTile*tiles*0.5,-height*0.5,-sizePerTile*tiles*0.5), glm::vec3(sizePerTile*tiles*0.5,height*0.5,sizePerTile*tiles*0.5));
	camera.position.x = mapBox.getCenter().x;
	camera.position.y = mapBox.getMaxY();
	camera.position.z = mapBox.getCenter().z;

	HeightMapTif heightFunction = HeightMapTif(filename, mapBox, sizePerTile,1.0f, -320.0f);
    CachedHeightMapSurface cache = CachedHeightMapSurface(heightFunction, mapBox, sizePerTile);
	HeightMap heightMap = HeightMap(cache, mapBox, sizePerTile);
	HeightMapDistanceFunction function(heightMap);
	WrappedHeightMap wrappedFunction = WrappedHeightMap(&function, minSize, model);
	solidSpace->add(wrappedFunction, model, DerivativeLandBrush(), minSize, simplifier, *solidSpaceChangeHandler);

	BoundingBox waterBox = mapBox;
	waterBox.setMaxY(0);
	
	//liquidSpace->add(OctreeContainmentHandler(solidSpace, waterBox, WaterBrush(0)),  minSize, simplifier);
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

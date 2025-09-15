
#include "tools.hpp"

Scene::Scene(Settings * settings, BrushContext * brushContext):
  
	solidSpace(BoundingCube(glm::vec3(0,0,0), 30.0), glm::pow(2, 9)),
	liquidSpace(BoundingCube(glm::vec3(0,0,0), 30.0), glm::pow(2, 9)),
	brushSpace(BoundingCube(glm::vec3(0,0,0), 30.0), glm::pow(2, 9)),
  	brushTrianglesCount(0),
	trianglesCount(0),
	brushContext(brushContext)

 {
	this->settings = settings;

	solidInstancesVisible = 0;
	liquidInstancesVisible = 0;
	vegetationInstancesVisible = 0;
	brushInstancesVisible = 0;
	debugInstancesVisible = 0;

	vegetationBuilder = new VegetationGeometryBuilder(new VegetationInstanceBuilderHandler(0.1, 4));
	debugBuilder = new OctreeGeometryBuilder(new OctreeInstanceBuilderHandler());
	meshBuilder = new MeshGeometryBuilder(&trianglesCount);

	solidRenderer = new OctreeVisibilityChecker(&visibleSolidNodes);
	brushRenderer = new OctreeVisibilityChecker(&visibleBrushNodes);
	liquidRenderer = new OctreeVisibilityChecker(&visibleLiquidNodes);
	for(int i = 0 ; i < SHADOW_MATRIX_COUNT ; ++i) {
		shadowRenderer[i]= new OctreeVisibilityChecker(&visibleShadowNodes[i]);
	}

	liquidSpaceChangeHandler = new LiquidSpaceChangeHandler(&liquidInfo);
	solidSpaceChangeHandler = new SolidSpaceChangeHandler(&vegetationInfo, &solidInfo);
	brushSpaceChangeHandler = new BrushSpaceChangeHandler(&brushInfo, &octreeWireframeInfo);

}

template <typename T> bool Scene::loadSpace(Octree* tree, OctreeNodeData& data, OctreeLayer<T>* infos, GeometryBuilder<T>* builder, ChunkContext * context) {
	bool emptyChunk = data.node->isEmpty() || data.node->isSolid();
	if(!emptyChunk){
		InstanceGeometry<T>* loadable = builder->build(tree, data, context);
		if (loadable == NULL) {
			// No geometry to load — erase entry if it exists
			infos->mutex.lock();
			infos->info.erase(data.node);
			infos->mutex.unlock();
			return false;
		}

		// Try to insert a new NodeInfo with loadable
		infos->mutex.lock();
		auto [it, inserted] = infos->info.try_emplace(data.node, loadable);
		infos->mutex.unlock();

		if (!inserted) {
			// Already existed — replace existing loadable
			if (it->second.loadable) {
				delete it->second.loadable;
			}
			it->second.loadable = loadable;
		}

		return true;
	}
	return false;
}


bool Scene::processLiquid(OctreeNodeData &data, Octree * tree) {
	bool result = false;
	ChunkContext context;
	if(loadSpace(tree, data, &liquidInfo, meshBuilder, &context)) {
		result = true;
	}
	data.node->setDirty(false);

	return result;
}

bool Scene::processSolid(OctreeNodeData &data, Octree * tree) {
	bool result = false;
	ChunkContext context;

	if(loadSpace(tree, data, &solidInfo, meshBuilder, &context)) {
		result = true;
	}
	if(loadSpace(tree, data, &vegetationInfo, vegetationBuilder, &context)) {
		result = true;			
	}
	#ifdef DEBUG_OCTREE_WIREFRAME
	if(loadSpace(tree, data, &octreeWireframeInfo, debugBuilder, &context)) {
		result = true;			
	}
	#endif
	data.node->setDirty(false);	

	return result;
}

bool Scene::processBrush(OctreeNodeData &data, Octree * tree) {
	bool result = false;
	ChunkContext context;

	if(loadSpace(tree, data, &brushInfo, meshBuilder, &context)) {
		result = true;
	}
	data.node->setDirty(false);	

	return result;
}

bool Scene::processSpace() {
	// Set load counts per Processor
	solidInstancesVisible = 0;
	liquidInstancesVisible = 0;
	vegetationInstancesVisible = 0;
	brushInstancesVisible = 0;
	debugInstancesVisible = 0;
	
	std::shared_ptr<std::atomic<int>> loadCountSolid = std::make_shared<std::atomic<int>>(0);
	std::shared_ptr<std::atomic<int>> loadCountLiquid = std::make_shared<std::atomic<int>>(0);
	std::shared_ptr<std::atomic<int>> loadCountBrush = std::make_shared<std::atomic<int>>(0);

	int maxSolidThreads = 8;
	int maxLiquidThreads = 8;
	int maxBrushThreads = 8;

	std::unordered_set<uint> visibleNodeIds;
	std::vector<OctreeNodeData*> allVisibleNodes;

	for(OctreeNodeData &data : visibleSolidNodes) {
		if(visibleNodeIds.find(data.node->id) == visibleNodeIds.end()) {
			allVisibleNodes.push_back(&data);
			visibleNodeIds.insert(data.node->id);
		}
	}

	for(int i =0 ; i < SHADOW_MATRIX_COUNT ; ++i) {
		std::vector<OctreeNodeData> &vec = visibleShadowNodes[i];
		for(OctreeNodeData &data : vec) {
			if(visibleNodeIds.find(data.node->id) == visibleNodeIds.end()) {
				allVisibleNodes.push_back(&data);
				visibleNodeIds.insert(data.node->id);
			}
		}
	}

    std::vector<std::thread> threads;
	threads.reserve(24);
	for(OctreeNodeData * data : allVisibleNodes) {
		if(data->node->isDirty() && --maxSolidThreads >= 0) {
			threads.emplace_back([this, data, loadCountSolid]() {
				if(processSolid(*data, &solidSpace)) {
					(*loadCountSolid)++;
				}
			});
		}
	}

	for(OctreeNodeData &data : visibleBrushNodes) {
		if(data.node->isDirty()  && --maxBrushThreads >= 0) {
			threads.emplace_back([this, &data, loadCountBrush]() {
				if(processBrush(data, &brushSpace)) {
					(*loadCountBrush)++;
				}
			});
		}
	}

	for(OctreeNodeData &data : visibleLiquidNodes) {
		if(data.node->isDirty() && --maxLiquidThreads >= 0) {
			threads.emplace_back([this, &data, loadCountLiquid]() {
				if(processLiquid(data, &liquidSpace)) {
					(*loadCountLiquid)++;
				}
			});
		}
	}

    for(std::thread &t : threads) {
        if(t.joinable()) {
            t.join();
        }
    }

	return *loadCountSolid > 0 || *loadCountLiquid > 0 || *loadCountBrush > 0;
}

void Scene::setVisibility(glm::mat4 viewProjection, std::vector<std::pair<glm::mat4, glm::vec3>> lightProjection ,Camera &camera) {
	setVisibleNodes(&solidSpace, viewProjection, camera.position, *solidRenderer);
	setVisibleNodes(&liquidSpace, viewProjection, camera.position, *liquidRenderer);
	setVisibleNodes(&brushSpace, viewProjection, camera.position, *brushRenderer);

	int i =0;
	for(std::pair<glm::mat4, glm::vec3> pair :  lightProjection){
		setVisibleNodes(&solidSpace, pair.first, pair.second, *shadowRenderer[i++]);
	}
}

void Scene::setVisibleNodes(Octree * tree, glm::mat4 viewProjection, glm::vec3 sortPosition, OctreeVisibilityChecker &checker) {
	checker.visibleNodes->clear();
	checker.sortPosition = sortPosition;
	checker.update(viewProjection);
	tree->iterateFlat(checker);	//here we get the visible nodes for that LOD + geometryLevel
}

template <typename T> DrawableInstanceGeometry<T> * Scene::loadIfNeeded(OctreeLayer<T>* infos, OctreeNode* node, InstanceHandler<T> * handler) {
	infos->mutex.lock();
	auto it = infos->info.find(node);
	auto end = infos->info.end();
	infos->mutex.unlock();
	if (it == end) {
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

template <typename T, typename H> void Scene::draw(uint drawableType, int mode, glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list, OctreeLayer<T> * info, long * count) {
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
	glDisable(GL_CULL_FACE);
	draw<DebugInstanceData, DebugInstanceDataHandler>(TYPE_INSTANCE_FULL_DRAWABLE, GL_TRIANGLES, cameraPosition, list, &octreeWireframeInfo, &debugInstancesVisible);
	glEnable(GL_CULL_FACE);
}

void Scene::generate(Camera &camera) {
	std::cout << "Scene::generate() " << std::endl;
	double startTime = glfwGetTime(); // Get elapsed time in seconds
	//WrappedSignedDistanceFunction::resetCalls();
	int sizePerTile = 30;
	int tiles= 256;
	int height = 2048;
	float minSize = 30;


	BoundingBox mapBox = BoundingBox(glm::vec3(-sizePerTile*tiles*0.5,-height*0.5,-sizePerTile*tiles*0.5), glm::vec3(sizePerTile*tiles*0.5,height*0.5,sizePerTile*tiles*0.5));
	camera.position.x = mapBox.getCenter().x;
	camera.position.y = mapBox.getMaxY();
	camera.position.z = mapBox.getCenter().z;



	{
		Transformation model = Transformation();
		std::cout << "\tGradientPerlinSurface"<< std::endl;
		GradientPerlinSurface heightFunction = GradientPerlinSurface(height, 1.0/(256.0f*sizePerTile), -64);
		std::cout << "\tCachedHeightMapSurface"<< std::endl;
		CachedHeightMapSurface cache = CachedHeightMapSurface(heightFunction, mapBox, sizePerTile);
		std::cout << "\tHeightMap"<< std::endl;
		HeightMap heightMap = HeightMap(cache, mapBox, sizePerTile);
		std::cout << "\tHeightMapDistanceFunction"<< std::endl;
		HeightMapDistanceFunction function = HeightMapDistanceFunction(&heightMap);
		std::cout << "\tWrappedHeightMap"<< std::endl;
		WrappedHeightMap wrappedFunction = WrappedHeightMap(&function);
		//wrappedFunction.cacheEnabled = true;
		
		std::cout << "\tsolidSpace.add(heightmap)"<< std::endl;
		solidSpace.add(&wrappedFunction, model, LandBrush(), minSize, *brushContext->simplifier, solidSpaceChangeHandler);
	}
	{
		std::cout << "\tsolidSpace.del(sphere)"<< std::endl;
		BoundingSphere sphere = BoundingSphere(glm::vec3(0,768,0),1024);
		SphereDistanceFunction function = SphereDistanceFunction();
		Transformation model = Transformation(glm::vec3(sphere.radius), sphere.center, 0, 0, 0);
		WrappedSphere wrappedFunction = WrappedSphere(&function);
		solidSpace.del(&wrappedFunction, model, SimpleBrush(15), minSize, *brushContext->simplifier, solidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.add(box)"<< std::endl;
		glm::vec3 min = glm::vec3(1500,0,500);
		glm::vec3 len = glm::vec3(512.0f);
		BoundingBox box = BoundingBox(min,min+len);
		BoxDistanceFunction function = BoxDistanceFunction();
		Transformation model = Transformation(box.getLength()*0.5f, box.getCenter(), 0, 0, 0);
		WrappedBox wrappedFunction = WrappedBox(&function);
		solidSpace.add(&wrappedFunction, model, SimpleBrush(9), minSize*2.0f, *brushContext->simplifier, solidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.add(sphere)"<< std::endl;
		glm::vec3 min = glm::vec3(1500,0,500);
		glm::vec3 len = glm::vec3(512.0f);
		BoundingSphere sphere = BoundingSphere(min+3.0f*len/4.0f, 256);
		SphereDistanceFunction function = SphereDistanceFunction();
		Transformation model = Transformation(glm::vec3(sphere.radius), sphere.center, 0, 0, 0);
		WrappedSphere wrappedFunction = WrappedSphere(&function);
		solidSpace.add(&wrappedFunction, model, SimpleBrush(7), minSize*0.5f, *brushContext->simplifier, solidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.del(sphere)"<< std::endl;
		glm::vec3 min = glm::vec3(1500,0,500);
		glm::vec3 len = glm::vec3(512.0f);
		BoundingSphere sphere = BoundingSphere(min+len, 128);
		SphereDistanceFunction function;
		Transformation model = Transformation(glm::vec3(sphere.radius), sphere.center, 0, 0, 0);
		WrappedSphere wrappedFunction = WrappedSphere(&function);
		solidSpace.del(&wrappedFunction, model, SimpleBrush(5), minSize*0.25f, *brushContext->simplifier, solidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.del(sphere)"<< std::endl;
		glm::vec3 min = glm::vec3(1500,0,500);
		glm::vec3 len = glm::vec3(512.0f);
		BoundingSphere sphere = BoundingSphere(min+3.0f*len/4.0f, 128);
		SphereDistanceFunction function = SphereDistanceFunction();
		Transformation model = Transformation(glm::vec3(sphere.radius), sphere.center, 0, 0, 0);
		WrappedSphere wrappedFunction = WrappedSphere(&function);
		solidSpace.del(&wrappedFunction, model, SimpleBrush(2), minSize, *brushContext->simplifier, solidSpaceChangeHandler);
	}

	{
		Transformation model = Transformation();
		std::cout << "\tsolidSpace.del(capsule)"<< std::endl;
		glm::vec3 a = glm::vec3(0,0, -3000);
		glm::vec3 b = glm::vec3(0,500,0);
		float r = 256.0f;
		CapsuleDistanceFunction function(a, b, r);
		WrappedCapsule wrappedFunction = WrappedCapsule(&function);
		solidSpace.del(&wrappedFunction, model, SimpleBrush(5), minSize, *brushContext->simplifier, solidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.add(sphere)"<< std::endl;
		glm::vec3 min = glm::vec3(1500,0,500);
		glm::vec3 len = glm::vec3(512.0f);
		BoundingSphere sphere = BoundingSphere(min+len, 64);
		SphereDistanceFunction function = SphereDistanceFunction();
		Transformation model = Transformation(glm::vec3(sphere.radius), sphere.center, 0, 0, 0);
		WrappedSphere wrappedFunction = WrappedSphere(&function);
		liquidSpace.add(&wrappedFunction, model, SimpleBrush(1), minSize*0.1f, *brushContext->simplifier, liquidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.add(octahedron)"<< std::endl;
		glm::vec3 center = glm::vec3(0,512, 512*0);
		float radius = 256.0f;
		OctahedronDistanceFunction function = OctahedronDistanceFunction();
		Transformation model = Transformation(glm::vec3(radius), center, 0, 0, 0);
		WrappedOctahedron wrappedFunction = WrappedOctahedron(&function);
		solidSpace.add(&wrappedFunction, model, SimpleBrush(5), minSize, *brushContext->simplifier, solidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.add(pyramid)"<< std::endl;
		glm::vec3 center = glm::vec3(0,512, 512*1);
		float radius = 256.0f;
		PyramidDistanceFunction function = PyramidDistanceFunction();
		Transformation model(glm::vec3(radius), center, 0,0,0);
		WrappedPyramid wrappedFunction = WrappedPyramid(&function);
		solidSpace.add(&wrappedFunction, model, SimpleBrush(5), minSize, *brushContext->simplifier, solidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.add(torus)"<< std::endl;
		glm::vec3 center = glm::vec3(0,512, 512*2);
		float radius = 256.0f;
		TorusDistanceFunction function = TorusDistanceFunction(glm::vec2(0.5, 0.25));
		Transformation model(glm::vec3(radius), center, 0,0,0);
		WrappedTorus wrappedFunction = WrappedTorus(&function);
		solidSpace.add(&wrappedFunction, model, SimpleBrush(5), minSize, *brushContext->simplifier, solidSpaceChangeHandler);
	}


	{
		std::cout << "\tsolidSpace.add(cone)"<< std::endl;
		glm::vec3 center = glm::vec3(0,512, 512*3);
		float radius = 256.0f;
		ConeDistanceFunction function = ConeDistanceFunction();
		Transformation model(glm::vec3(radius), center, 0,0,0);
		WrappedCone wrappedFunction = WrappedCone(&function);
		solidSpace.add(&wrappedFunction, model, SimpleBrush(5), minSize, *brushContext->simplifier, solidSpaceChangeHandler);
	}


	{
		Transformation model = Transformation();
		std::cout << "\tsolidSpace.add(water)"<< std::endl;
		BoundingBox waterBox = mapBox;
		waterBox.setMaxY(0);
		OctreeDifferenceFunction function(&solidSpace, waterBox);
		WrappedOctreeDifference wrappedFunction = WrappedOctreeDifference(&function);
		//wrappedFunction.cacheEnabled = true;
		liquidSpace.add(&wrappedFunction, model, WaterBrush(1), minSize, *brushContext->simplifier, liquidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.add(box)"<< std::endl;
		glm::vec3 min = glm::vec3(1500,0,-1000);
		glm::vec3 len = glm::vec3(512.0f);
		BoundingBox box = BoundingBox(min,min+len);
		BoxDistanceFunction function = BoxDistanceFunction();
		Transformation model = Transformation(box.getLength()*0.5f, box.getCenter(), 0, 0, 0);
		WrappedBox wrappedFunction = WrappedBox(&function);
		solidSpace.add(&wrappedFunction, model, SimpleBrush(9), minSize*4, *brushContext->simplifier, solidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.add(box)"<< std::endl;
		glm::vec3 min = glm::vec3(2500,0,-1000);
		glm::vec3 len = glm::vec3(512.0f);
		BoundingBox box = BoundingBox(min,min+len);
		BoxDistanceFunction function = BoxDistanceFunction();
		Transformation model = Transformation(box.getLength()*0.5f, box.getCenter(), 0, 0, 0);
		WrappedBox wrappedFunction = WrappedBox(&function);
		solidSpace.add(&wrappedFunction, model, SimpleBrush(9), minSize*0.25, *brushContext->simplifier, solidSpaceChangeHandler);
	}



	//exportOctree();

	double endTime = glfwGetTime(); // Get elapsed time in seconds
	//std::cout << "Scene::callsToSDF " << std::to_string(WrappedSignedDistanceFunction::_calls)   << std::endl;

	std::cout << "Scene::generate Ok! " << std::to_string(endTime-startTime) << "s"  << std::endl;
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
	HeightMapDistanceFunction function = HeightMapDistanceFunction(&heightMap);
	WrappedHeightMap wrappedFunction = WrappedHeightMap(&function);
	solidSpace.add(&wrappedFunction, model, DerivativeLandBrush(), minSize, *brushContext->simplifier, solidSpaceChangeHandler);

	BoundingBox waterBox = mapBox;
	waterBox.setMaxY(0);
	
	//exportOctree();
}

void Scene::save(std::string folderPath, Camera &camera) {
	OctreeFile saver1(&solidSpace, "solid");
	OctreeFile saver2(&liquidSpace, "liquid");
	SettingsFile settingsFile(settings, "settings");
	settingsFile.save(folderPath);
	saver1.save(folderPath, 4096);
	saver2.save(folderPath, 4096);
}

void Scene::load(std::string folderPath, Camera &camera) {
	OctreeFile loader1(&solidSpace, "solid");
	OctreeFile loader2(&liquidSpace, "liquid");
	SettingsFile settingsFile(settings, "settings");
	settingsFile.load(folderPath);
	loader1.load(folderPath, 4096);
	loader2.load(folderPath, 4096);
	//camera.position.x = loader1.getBox().getCenter().x;
	//camera.position.y = loader1.getBox().getMaxY();
	//camera.position.z = loader1.getBox().getCenter().z;
}

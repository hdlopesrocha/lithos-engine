
#include "tools.hpp"

// Random number generator
std::random_device rd;
std::mt19937 g(rd());  // Mersenne Twister engine

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
	debugBuilder = new OctreeGeometryBuilder(new OctreeInstanceBuilderHandler());


	
	solidRenderer = new OctreeVisibilityChecker();
	brushRenderer = new OctreeVisibilityChecker();
	liquidRenderer = new OctreeVisibilityChecker();
	for(int i = 0 ; i < SHADOW_MATRIX_COUNT ; ++i) {
		shadowRenderer[i]= new OctreeVisibilityChecker();
	}

	liquidSpaceChangeHandler = new LiquidSpaceChangeHandler(&liquidInfo);
	solidSpaceChangeHandler = new SolidSpaceChangeHandler(&vegetationInfo);
	brushSpaceChangeHandler = new BrushSpaceChangeHandler(&brushInfo, &octreeWireframeInfo);
	vegetationGeometry = new Vegetation3d(1.0);
}

template <typename T> bool Scene::loadSpace(Octree* tree, OctreeNodeData& data, OctreeLayer<T>* infos, InstanceGeometry<T>* loadable) {
	bool isSurface = !data.node->isEmpty() && !data.node->isSolid();

	if(isSurface){
		if (loadable == NULL) {
			std::unique_lock(infos->mutex2);
			// No geometry to load — erase entry if it exists
			//TODO check thread safety
			infos->info.erase(data.node);
			return false;
		}
		using MapType   = std::unordered_map<OctreeNode*, NodeInfo<T>>;
		using Iterator  = typename MapType::iterator;

		// Try to insert a new NodeInfo with loadable
		std::unique_lock(infos->mutex2);

		std::pair<Iterator, bool> iter = infos->info.try_emplace(data.node, loadable);
		Iterator it = iter.first;
		bool inserted = iter.second;
	
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
	
	ThreadContext context;
	Tesselator tesselator(&trianglesCount, &context);
	std::vector<OctreeNodeTriangleHandler*> handlers;
	handlers.emplace_back(&tesselator);
	Processor processor(&trianglesCount, &context, &handlers);
	processor.iterate(tree, &data);

 	if(tesselator.geometry->indices.size() > 0) {
        InstanceGeometry<InstanceData> * pre = new InstanceGeometry<InstanceData>(tesselator.geometry);
        pre->instances.emplace_back(InstanceData(0, glm::mat4(1.0), 0.0f));

		if(loadSpace(tree, data, &liquidInfo, pre)) {
			result = true;
		}
	}else {
		if(loadSpace(tree, data, &liquidInfo, (InstanceGeometry<InstanceData>*) NULL)) {
			result = true;
		}
	}
	
	if(data.node) {
		data.node->setDirty(false);
	}
	return result;
}




bool Scene::processSolid(OctreeNodeData &data, Octree * tree) {
	//std::cout << "processSolid " << std::to_string((long)&data.node) <<  std::endl;

	bool result = false;
	ThreadContext context = ThreadContext(data.cube);
	Tesselator tesselator(&trianglesCount, &context);
	std::vector<InstanceData> vegetationInstances; 
	long count = 0;
	VegetationInstanceBuilder vegetationBuilder(tree, &count, &vegetationInstances, 0.1, 4);

	std::vector<OctreeNodeTriangleHandler*> handlers;
	handlers.emplace_back(&tesselator);
	handlers.emplace_back(&vegetationBuilder);
	//std::cout << "\tprocessor" << std::endl;
	Processor processor(&trianglesCount, &context, &handlers);
	//std::cout << "\tprocessor.iterateFlatIn" << std::endl;
	processor.iterate(tree, &data);

 	if(tesselator.geometry->indices.size() > 0) {
        InstanceGeometry<InstanceData> * pre = new InstanceGeometry<InstanceData>(tesselator.geometry);
        pre->instances.emplace_back(InstanceData(0, glm::mat4(1.0), 0.0f));
		//std::cout << "\tloadSpace(solidInfo) " << tesselator.geometry->indices.size() <<  std::endl;

		if(data.node && loadSpace(tree, data, &solidInfo, pre)) {
			result = true;
		}

	} else {
		if(data.node && loadSpace(tree, data, &solidInfo, (InstanceGeometry<InstanceData>*) NULL)) {
			result = true;
		}
	}

	//std::cout << "\tshuffle " << vegetationInstances.size() <<  std::endl;
	
    // Shuffle the vector
    if(vegetationInstances.size()) {
        std::shuffle(vegetationInstances.begin(), vegetationInstances.end(), g);
        InstanceGeometry<InstanceData> * pre = new InstanceGeometry<InstanceData>(vegetationGeometry, vegetationInstances);
		//std::cout << "\tloadSpace(vegetationInfo) " << tesselator.geometry->indices.size() <<  std::endl;
		if(data.node && loadSpace(tree, data, &vegetationInfo, pre)) {
			result = true;
		}
    } else {
		if(data.node && loadSpace(tree, data, &vegetationInfo, (InstanceGeometry<InstanceData>*) NULL)) {
			result = true;
		}
	}
	
	
	#ifdef DEBUG_OCTREE_WIREFRAME
	auto debugInstances = debugBuilder->build(tree, data, &context);
	if(data.node && loadSpace(tree, data, &octreeWireframeInfo, debugInstances)) {
		result = true;			
	}
	#endif
	
	
	//	std::cout << "\tnode.setDirty " << std::to_string((long) data.node) <<  std::endl;

	if(data.node) {
		data.node->setDirty(false);	
	}

	return result;
}

bool Scene::processBrush(OctreeNodeData &data, Octree * tree) {
	bool result = false;
	
	ThreadContext context;
	Tesselator tesselator(&trianglesCount, &context);

	std::vector<OctreeNodeTriangleHandler*> handlers;
	handlers.emplace_back(&tesselator);
	Processor processor(&trianglesCount, &context, &handlers);
	processor.iterate(tree, &data);

 	if(tesselator.geometry->indices.size() > 0) {
        InstanceGeometry<InstanceData> * pre = new InstanceGeometry<InstanceData>(tesselator.geometry);
        pre->instances.emplace_back(InstanceData(0, glm::mat4(1.0), 0.0f));

		if(data.node && loadSpace(tree, data, &brushInfo, pre)) {
			result = true;
		}
	}else {
		if(data.node && loadSpace(tree, data, &brushInfo, (InstanceGeometry<InstanceData>*) NULL)) {
			result = true;
		}
	}
	
	if(data.node) {
		data.node->setDirty(false);	
	}
	return result;
}

bool Scene::processSpace() {
	// Set load counts per Processor

	std::unordered_set<uint> visibleNodeIds;
	std::vector<OctreeNodeData*> allVisibleNodes;

	for(OctreeNodeData &data : solidRenderer->visibleNodes) {
		if(data.node && data.node->id != UINT_MAX && visibleNodeIds.find(data.node->id) == visibleNodeIds.end()) {
			allVisibleNodes.emplace_back(&data);
			visibleNodeIds.insert(data.node->id);
		}
	}

	for(uint i =0 ; i < SHADOW_MATRIX_COUNT ; ++i) {
		std::vector<OctreeNodeData> &vec = shadowRenderer[i]->visibleNodes;
		for(OctreeNodeData &data : vec) {
			if(data.node && data.node->id != UINT_MAX && visibleNodeIds.find(data.node->id) == visibleNodeIds.end()) {
				allVisibleNodes.emplace_back(&data);
				visibleNodeIds.insert(data.node->id);
			}
		}
	}

	int loadCount = 0;

	//std::cout << "process " << std::to_string((long)allVisibleNodes.size()) <<  std::endl;

    std::vector<std::future<bool>> threads;
	threads.reserve(64);

	// Thread pool zone
	
	for (OctreeNodeData* data : allVisibleNodes) {
		if (data->node && data->node->isDirty()) {
			threads.emplace_back(threadPool.enqueue([this, data]() {
				return processSolid(*data, &solidSpace);
			}));
		}
	}

	for (OctreeNodeData& brush : brushRenderer->visibleNodes) {
		if (brush.node && brush.node->isDirty()) {
			threads.emplace_back(threadPool.enqueue([this, &brush]() {
				return processBrush(brush, &brushSpace);
			}));
		}
	}

	for (OctreeNodeData& liquid : liquidRenderer->visibleNodes) {
		if (liquid.node && liquid.node->isDirty()) {
			threads.emplace_back(threadPool.enqueue([this, &liquid]() {
				return processLiquid(liquid, &liquidSpace);
			}));
		}
	}
	for(auto &t : threads) {
		bool ret = t.get();
		if(ret) {
			++loadCount;
		}
	}

	return loadCount > 0;
}

void Scene::setVisibility(glm::mat4 viewProjection, std::vector<std::pair<glm::mat4, glm::vec3>> lightProjection ,Camera &camera) {
	setVisibleNodes(&solidSpace, viewProjection, camera.position, solidRenderer);
	setVisibleNodes(&liquidSpace, viewProjection, camera.position, liquidRenderer);
	setVisibleNodes(&brushSpace, viewProjection, camera.position, brushRenderer);

	int i =0;
	for(std::pair<glm::mat4, glm::vec3> pair :  lightProjection){
		setVisibleNodes(&solidSpace, pair.first, pair.second, shadowRenderer[i++]);
	}
}

void Scene::setVisibleNodes(Octree * tree, glm::mat4 viewProjection, glm::vec3 sortPosition, OctreeVisibilityChecker * checker) {
	checker->visibleNodes.clear();
	//checker.visibleNodes->reserve(128);
	checker->sortPosition = sortPosition;
	checker->update(viewProjection);
	//TODO change to best iterator
	tree->iterate(*checker);	//here we get the visible nodes for that LOD + geometryLevel
}

template <typename T> DrawableInstanceGeometry<T> * Scene::loadIfNeeded(OctreeLayer<T>* infos, OctreeNode* node, InstanceHandler<T> * handler) {
	auto it = infos->info.find(node);
	auto end = infos->info.end();
	if (it == end) {
		return NULL;
	}
	NodeInfo<T>& ni = it->second;
	if (ni.loadable) {
		std::cout << "Scene::loadIfNeeded " << std::to_string((long)ni.loadable) << std:: endl;

		if (ni.drawable) {
			delete ni.drawable;
		}
		ni.drawable = new DrawableInstanceGeometry<T>(ni.loadable->geometry, &ni.loadable->instances, handler);
		delete ni.loadable;
		ni.loadable = NULL;
	}
	
	return ni.drawable;
}

template <typename T, typename H>
void Scene::draw(uint drawableType, int mode, glm::vec3 cameraPosition,
                 const OctreeVisibilityChecker* checker,
                 OctreeLayer<T>* info, long* count) {
    H handler;
    if (checker == NULL) return;

    for (const auto& data : checker->visibleNodes) {
        if (!data.node) continue;

        OctreeNode* node = data.node;
        DrawableInstanceGeometry<T>* drawable = loadIfNeeded(info, node, &handler);
        if (!drawable) continue;
		
        if (drawableType == TYPE_INSTANCE_AMOUNT_DRAWABLE) {
            float amount = glm::clamp(
                1.0f - glm::length(cameraPosition - drawable->center) /
                        static_cast<float>(settings->billboardRange),
                0.0f, 1.0f
            );
            if (amount > 0.8f) amount = 1.0f;
            drawable->draw(mode, amount, count);
        }
        else if (drawableType == TYPE_INSTANCE_FULL_DRAWABLE) {
            drawable->draw(mode, 1.0f, count);
        }
    }
}

void Scene::drawVegetation(glm::vec3 cameraPosition, const OctreeVisibilityChecker * checker) {
	glDisable(GL_CULL_FACE);
	vegetationInstancesVisible = 0;
	draw<InstanceData, InstanceDataHandler>(TYPE_INSTANCE_AMOUNT_DRAWABLE, GL_PATCHES, cameraPosition, checker, &vegetationInfo, &vegetationInstancesVisible);
	glEnable(GL_CULL_FACE);
}

void Scene::draw3dSolid(glm::vec3 cameraPosition, const OctreeVisibilityChecker * checker) {
	solidInstancesVisible = 0;
	draw<InstanceData, InstanceDataHandler>(TYPE_INSTANCE_FULL_DRAWABLE, GL_PATCHES, cameraPosition, checker, &solidInfo, &solidInstancesVisible);
}

void Scene::draw3dBrush(glm::vec3 cameraPosition, const OctreeVisibilityChecker * checker) {
	brushInstancesVisible = 0;
	draw<InstanceData, InstanceDataHandler>(TYPE_INSTANCE_FULL_DRAWABLE, GL_PATCHES, cameraPosition, checker, &brushInfo, &brushInstancesVisible);
}

void Scene::draw3dLiquid(glm::vec3 cameraPosition, const OctreeVisibilityChecker * checker) {
	liquidInstancesVisible = 0;
	draw<InstanceData, InstanceDataHandler>(TYPE_INSTANCE_FULL_DRAWABLE, GL_PATCHES, cameraPosition, checker, &liquidInfo, &liquidInstancesVisible);
}

void Scene::draw3dOctree(glm::vec3 cameraPosition, const OctreeVisibilityChecker * checker) {
	glDisable(GL_CULL_FACE);
	debugInstancesVisible = 0;
	draw<DebugInstanceData, DebugInstanceDataHandler>(TYPE_INSTANCE_FULL_DRAWABLE, GL_TRIANGLES, cameraPosition, checker, &octreeWireframeInfo, &debugInstancesVisible);
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
		WrappedPerlinDistortDistanceEffect distortedFunction = WrappedPerlinDistortDistanceEffect(&wrappedFunction, 64.0f, 0.1f/32.0f, glm::vec3(0), 0.0f, 1.0f);
		solidSpace.del(&distortedFunction, model, SimpleBrush(5), minSize, *brushContext->simplifier, solidSpaceChangeHandler);
	}

	{
		std::cout << "\tliquidSpace.add(sphere)"<< std::endl;
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
		std::cout << "\tsolidSpace.add(cylinder)"<< std::endl;
		glm::vec3 center = glm::vec3(0,512, 512*4);
		float radius = 256.0f;
		CylinderDistanceFunction function = CylinderDistanceFunction();
		Transformation model(glm::vec3(radius), center, 0,0,0);
		WrappedCylinder wrappedFunction = WrappedCylinder(&function);
		solidSpace.add(&wrappedFunction, model, SimpleBrush(5), minSize, *brushContext->simplifier, solidSpaceChangeHandler);
	}


	{
		std::cout << "\tsolidSpace.add(perlinDistort)"<< std::endl;
		glm::vec3 center = glm::vec3(512,512, 512*0);
		float radius = 200.0f;
		SphereDistanceFunction function = SphereDistanceFunction();
		Transformation model(glm::vec3(radius), center, 0,0,0);
		WrappedSphere wrappedFunction = WrappedSphere(&function);
		WrappedPerlinDistortDistanceEffect distortedFunction = WrappedPerlinDistortDistanceEffect(&wrappedFunction, 48.0f, 0.1f/32.0f, glm::vec3(0), 0.0f, 1.0f);
		//distortedFunction.cacheEnabled = true;
		solidSpace.add(&distortedFunction, model, SimpleBrush(5), minSize*0.25f, *brushContext->simplifier, solidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.add(perlinCarve)"<< std::endl;
		glm::vec3 center = glm::vec3(512,512, 512*1);
		float radius = 200.0f;
		SphereDistanceFunction function = SphereDistanceFunction();
		Transformation model(glm::vec3(radius), center, 0,0,0);
		WrappedSphere wrappedFunction = WrappedSphere(&function);
		WrappedPerlinCarveDistanceEffect carvedFunction = WrappedPerlinCarveDistanceEffect(&wrappedFunction, 64.0f, 0.1f/32.0f, 0.1f, glm::vec3(0), 0.0f, 1.0f);
		//carvedFunction.cacheEnabled = true;
		solidSpace.add(&carvedFunction, model, SimpleBrush(5), minSize*0.2f, *brushContext->simplifier, solidSpaceChangeHandler);
	}
	{
		std::cout << "\tsolidSpace.add(sineDistort)"<< std::endl;
		glm::vec3 center = glm::vec3(512,512, 512*2);
		float radius = 200.0f;
		SphereDistanceFunction function = SphereDistanceFunction();
		Transformation model(glm::vec3(radius), center, 0,0,0);
		WrappedSphere wrappedFunction = WrappedSphere(&function);
		WrappedSineDistortDistanceEffect carvedFunction = WrappedSineDistortDistanceEffect(&wrappedFunction, 32.0f, 0.1f/2.0f, glm::vec3(0));
		//carvedFunction.cacheEnabled = true;
		solidSpace.add(&carvedFunction, model, SimpleBrush(5), minSize*0.25f, *brushContext->simplifier, solidSpaceChangeHandler);
	}
	{
		std::cout << "\tsolidSpace.add(voronoiDistort)"<< std::endl;
		glm::vec3 center = glm::vec3(512,512, 512*3);
		float radius = 200.0f;
		SphereDistanceFunction function = SphereDistanceFunction();
		Transformation model(glm::vec3(radius), center, 0,0,0);
		WrappedSphere wrappedFunction = WrappedSphere(&function);
		WrappedVoronoiCarveDistanceEffect distortFunction = WrappedVoronoiCarveDistanceEffect(&wrappedFunction, 64.0f, 64.0f, glm::vec3(0), 0.0f, 1.0f);
		solidSpace.add(&distortFunction, model, SimpleBrush(5), minSize*0.25f, *brushContext->simplifier, solidSpaceChangeHandler);
	}
	{
		std::cout << "\tsolidSpace.add(voronoiDistort)"<< std::endl;
		glm::vec3 center = glm::vec3(512,512, 512*4);
		float radius = 200.0f;
		SphereDistanceFunction function = SphereDistanceFunction();
		Transformation model(glm::vec3(radius), center, 0,0,0);
		WrappedSphere wrappedFunction = WrappedSphere(&function);
		WrappedVoronoiCarveDistanceEffect distortFunction = WrappedVoronoiCarveDistanceEffect(&wrappedFunction, 64.0f, 64.0f, glm::vec3(0), 0.0f, -1.0f);
		solidSpace.add(&distortFunction, model, SimpleBrush(5), minSize*0.25f, *brushContext->simplifier, solidSpaceChangeHandler);
	}
	{
		Transformation model = Transformation();
		std::cout << "\tliquidSpace.add(water)"<< std::endl;
		BoundingBox waterBox = mapBox;
		waterBox.setMaxY(0);
		waterBox.setMinY(mapBox.getMinY()*0.5f);
		OctreeDifferenceFunction function(&solidSpace, waterBox, minSize*2.0f);
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

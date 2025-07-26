
#include "tools.hpp"

Scene::Scene(Settings * settings, ComputeShader * computeShader):
  
	solidSpace(BoundingCube(glm::vec3(0,0,0), 30.0), glm::pow(2, 9)),
	liquidSpace(BoundingCube(glm::vec3(0,0,0), 30.0), glm::pow(2, 9)),
	brushSpace(BoundingCube(glm::vec3(0,0,0), 30), glm::pow(2, 9)),
  	brushTrianglesCount(0),
	solidTrianglesCount(0),
	liquidTrianglesCount(0),
	simplifier(0.99, 0.1, true), 
	computeShader(computeShader)

 {
	this->settings = settings;

	solidInstancesVisible = 0;
	liquidInstancesVisible = 0;
	vegetationInstancesVisible = 0;
	brushInstancesVisible = 0;
	debugInstancesVisible = 0;

	vegetationBuilder = new VegetationGeometryBuilder(&solidSpace, new VegetationInstanceBuilderHandler(&solidSpace, 0.1, 4));
	debugBuilder = new OctreeGeometryBuilder(new OctreeInstanceBuilderHandler());

	solidRenderer = new OctreeVisibilityChecker(&visibleSolidNodes);
	brushRenderer = new OctreeVisibilityChecker(&visibleBrushNodes);
	liquidRenderer = new OctreeVisibilityChecker(&visibleLiquidNodes);
	for(int i = 0 ; i < SHADOW_MATRIX_COUNT ; ++i) {
		shadowRenderer[i]= new OctreeVisibilityChecker(&visibleShadowNodes[i]);
	}

	liquidSpaceChangeHandler = LiquidSpaceChangeHandler(&liquidInfo);
	solidSpaceChangeHandler = SolidSpaceChangeHandler(&vegetationInfo, &debugInfo, &solidInfo);
	brushSpaceChangeHandler = BrushSpaceChangeHandler(&brushInfo, &debugInfo);

	inputSSBO.allocate();
	outputSSBO.allocate();
	octreeSSBO.allocate();
}

template <typename T> bool Scene::loadSpace(Octree* tree, OctreeNodeData& data, std::unordered_map<OctreeNode*, NodeInfo<T>>* infos, GeometryBuilder<T>* builder) {
	bool emptyChunk = data.node->isEmpty() || data.node->isSolid();
	if(!emptyChunk){
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
	return false;
}

bool Scene::computeGeometry(OctreeNodeData &data, Octree * tree, std::unordered_map<OctreeNode*, GeometrySSBO>* infos) {
	BoundingCube chunkBox = data.cube;
	bool emptyChunk = data.node->isEmpty() || data.node->isSolid();
    //std::cout << "Scene::computeGeometry() " << "minX:" << chunkBox.getMinX() << ", minY:" << chunkBox.getMinY() << ", minZ:" << chunkBox.getMinZ() << ", len:" << chunkBox.getLengthX() << std::endl;
	if (!emptyChunk)	{

		std::vector<OctreeNodeCubeSerialized> nodes;
		nodes.reserve(2000);
		BoundingCube chunkOverlap = data.cube;
		chunkOverlap.setLength(chunkOverlap.getLengthX()*1.1f);
		tree->root->exportSerialization(&tree->allocator, &nodes, *tree, chunkOverlap, true);


		(*infos)[data.node] = GeometrySSBO();
		octreeSSBO.copy(&nodes);

		GeometrySSBO &ssbo = (*infos)[data.node];
		ssbo.allocate(nodes.size());

		inputSSBO.copy(ComputeShaderInput(chunkBox.getMin(), chunkBox.getLength())); 
		outputSSBO.reset();
		computeShader->dispatch(octreeSSBO.nodesCount);

		ComputeShaderOutput result = outputSSBO.read();
		ssbo.vertexCount = result.vertexCount;
		ssbo.indexCount = result.indexCount;

		if (result.vertexCount == 0 || result.indexCount == 0) {
			return false;
		}
		/*
		std::cout << "\tresult4f0 = { " 
			<< std::to_string(result.result4f0.x) << ", " 
			<< std::to_string(result.result4f0.y) << ", " 
			<< std::to_string(result.result4f0.z) << ", " 
			<< std::to_string(result.result4f0.w) << " }"  << std::endl;
		std::cout << "\tresult4f1 = { " 
			<< std::to_string(result.result4f1.x) << ", " 
			<< std::to_string(result.result4f1.y) << ", " 
			<< std::to_string(result.result4f1.z) << ", " 
			<< std::to_string(result.result4f1.w) << " }"  << std::endl;

		std::cout << "\tvertexCount = " << std::to_string(result.vertexCount) <<std::endl;
		std::cout << "\tindexCount = " << std::to_string(result.indexCount) <<std::endl;
		*/
		return true;
	}
	return false;
}

bool Scene::processLiquid(OctreeNodeData &data, Octree * tree) {
	bool result = false;
	if(data.node->isDirty()) {
		if(computeGeometry(data, tree, &liquidInfo)) {
			result = true;
		}
		data.node->setDirty(false);
	}
	return result;
}

bool Scene::processSolid(OctreeNodeData &data, Octree * tree) {
	bool result = false;
	if(data.node->isDirty()) { 
		if(computeGeometry(data, tree, &solidInfo)) {
			result = true;
		}
		if(loadSpace(tree, data, &vegetationInfo, vegetationBuilder)) {
			result = true;			
		}
		data.node->setDirty(false);	
	}
	return result;
}

bool Scene::processBrush(OctreeNodeData &data, Octree * tree) {
	bool result = false;
	if(data.node->isDirty()) { 
		if(computeGeometry(data, tree, &brushInfo)) {
			result = true;
		}
		if(loadSpace(tree, data, &debugInfo, debugBuilder)) {
			result = true;			
		}
		data.node->setDirty(false);	
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
	int loadCountSolid = 10;
	int loadCountLiquid = 1;

	for(OctreeNodeData &data : visibleSolidNodes) {
		if(loadCountSolid > 0) {
			if(processSolid(data, &solidSpace)) {
				--loadCountSolid;
			}
		} else {
			break;
		}
	}

	for(OctreeNodeData &data : visibleBrushNodes) {
		processBrush(data, &brushSpace);
	}

	for(OctreeNodeData &data : visibleLiquidNodes) {
		if(loadCountLiquid > 0) {
			if(processLiquid(data, &liquidSpace)) {
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
				if(processSolid(data, &solidSpace)) {
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

void drawGeometry(const std::vector<OctreeNodeData> &list, std::unordered_map<OctreeNode*, GeometrySSBO> * infos ) {
	for(const OctreeNodeData &data : list) {
		OctreeNode * node = data.node;
		auto it = infos->find(node);
		if (it != infos->end()) {
			GeometrySSBO* geo = &it->second;
			if(geo != NULL && geo->vertexCount > 0 && geo->indexCount > 0 && geo->vertexArrayObject > 0) {
				//std::cout << "Scene::draw3dSolid() " << std::to_string(geo->vertexCount) << " vertices, " << std::to_string(geo->indexCount) << " indices, " << std::to_string(geo->vertexArrayObject) << " vao" << std::endl;
				glBindVertexArray(geo->vertexArrayObject);
				glDrawElementsInstanced(GL_PATCHES, geo->indexCount, GL_UNSIGNED_INT, nullptr, 1);
				glBindVertexArray(0);
			}
		}
	} 
}


void Scene::draw3dSolid(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) {
	drawGeometry(list, &solidInfo);
}

void Scene::draw3dBrush(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) {
	drawGeometry(list, &brushInfo);
}

void Scene::draw3dLiquid(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) {
	drawGeometry(list, &liquidInfo);
}

void Scene::draw3dOctree(glm::vec3 cameraPosition, const std::vector<OctreeNodeData> &list) {
	draw<DebugInstanceData, DebugInstanceDataHandler>(TYPE_INSTANCE_FULL_DRAWABLE, GL_LINES, cameraPosition, list, &debugInfo, &debugInstancesVisible);
}

void Scene::generate(Camera &camera) {
	std::cout << "Scene::generate() " << std::endl;
	double startTime = glfwGetTime(); // Get elapsed time in seconds
	//WrappedSignedDistanceFunction::resetCalls();
	int sizePerTile = 30;
	int tiles= 256;
	int height = 2048;
	float minSize = 30;
	Transformation model = Transformation();


	BoundingBox mapBox = BoundingBox(glm::vec3(-sizePerTile*tiles*0.5,-height*0.5,-sizePerTile*tiles*0.5), glm::vec3(sizePerTile*tiles*0.5,height*0.5,sizePerTile*tiles*0.5));
	camera.position.x = mapBox.getCenter().x;
	camera.position.y = mapBox.getMaxY();
	camera.position.z = mapBox.getCenter().z;



	{
		std::cout << "\tGradientPerlinSurface"<< std::endl;
		GradientPerlinSurface heightFunction = GradientPerlinSurface(height, 1.0/(256.0f*sizePerTile), -64);
		std::cout << "\tCachedHeightMapSurface"<< std::endl;
		CachedHeightMapSurface cache = CachedHeightMapSurface(heightFunction, mapBox, sizePerTile);
		std::cout << "\tHeightMap"<< std::endl;
		HeightMap heightMap = HeightMap(cache, mapBox, sizePerTile);
		std::cout << "\tHeightMapDistanceFunction"<< std::endl;
		HeightMapDistanceFunction function(heightMap);
		std::cout << "\tWrappedHeightMap"<< std::endl;
		WrappedHeightMap wrappedFunction = WrappedHeightMap(&function, minSize, model);
		//wrappedFunction.cacheEnabled = true;
		
		std::cout << "\tsolidSpace.add(heightmap)"<< std::endl;
		solidSpace.add(wrappedFunction, model, LandBrush(), minSize, simplifier, &solidSpaceChangeHandler);
	}
	{
		std::cout << "\tsolidSpace.del(sphere)"<< std::endl;
		BoundingSphere sphere = BoundingSphere(glm::vec3(0,768,0),1024);
		SphereDistanceFunction function(sphere.center, sphere.radius);
		WrappedSphere wrappedFunction = WrappedSphere(&function, minSize, model);
		solidSpace.del(wrappedFunction, model, SimpleBrush(14), minSize, simplifier, &solidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.add(box)"<< std::endl;
		glm::vec3 min = glm::vec3(1500,0,500);
		glm::vec3 len = glm::vec3(512.0f);
		BoundingBox box = BoundingBox(min,min+len);
		BoxDistanceFunction function(box.getCenter(), box.getLength()*0.5f);
		WrappedBox wrappedFunction = WrappedBox(&function, minSize, model);
		solidSpace.add(wrappedFunction, model, SimpleBrush(8), minSize, simplifier, &solidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.add(sphere)"<< std::endl;
		glm::vec3 min = glm::vec3(1500,0,500);
		glm::vec3 len = glm::vec3(512.0f);
		BoundingSphere sphere = BoundingSphere(min+3.0f*len/4.0f, 256);
		SphereDistanceFunction function(sphere.center, sphere.radius);
		WrappedSphere wrappedFunction = WrappedSphere(&function, minSize, model);
		solidSpace.add(wrappedFunction, model, SimpleBrush(6), minSize, simplifier, &solidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.del(sphere)"<< std::endl;
		glm::vec3 min = glm::vec3(1500,0,500);
		glm::vec3 len = glm::vec3(512.0f);
		BoundingSphere sphere = BoundingSphere(min+len, 128);
		SphereDistanceFunction function(sphere.center, sphere.radius);
		WrappedSphere wrappedFunction = WrappedSphere(&function, minSize, model);
		solidSpace.del(wrappedFunction, model, SimpleBrush(4), minSize, simplifier, &solidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.del(sphere)"<< std::endl;
		glm::vec3 min = glm::vec3(1500,0,500);
		glm::vec3 len = glm::vec3(512.0f);
		BoundingSphere sphere = BoundingSphere(min+3.0f*len/4.0f, 128);
		SphereDistanceFunction function(sphere.center, sphere.radius);
		WrappedSphere wrappedFunction = WrappedSphere(&function, minSize, model);
		solidSpace.del(wrappedFunction, model, SimpleBrush(1), minSize, simplifier, &solidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.del(capsule)"<< std::endl;
		glm::vec3 a = glm::vec3(0,0, -3000);
		glm::vec3 b = glm::vec3(0,500,0);
		float r = 256.0f;
		CapsuleDistanceFunction function(a, b, r);
		WrappedCapsule wrappedFunction = WrappedCapsule(&function, minSize, model);
		solidSpace.del(wrappedFunction, model, SimpleBrush(4), minSize, simplifier, &solidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.add(sphere)"<< std::endl;
		glm::vec3 min = glm::vec3(1500,0,500);
		glm::vec3 len = glm::vec3(512.0f);
		BoundingSphere sphere = BoundingSphere(min+len, 64);
		SphereDistanceFunction function(sphere.center, sphere.radius);
		WrappedSphere wrappedFunction = WrappedSphere(&function, minSize, model);
		liquidSpace.add(wrappedFunction, model, SimpleBrush(0), minSize, simplifier, &liquidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.add(octahedron)"<< std::endl;
		glm::vec3 center = glm::vec3(0,400,-600);
		float radius = 256.0f;
		OctahedronDistanceFunction function(center, radius);
		WrappedOctahedron wrappedFunction = WrappedOctahedron(&function, minSize, model);
		solidSpace.add(wrappedFunction, model, SimpleBrush(4), minSize, simplifier, &solidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.add(pyramid)"<< std::endl;
		glm::vec3 center = glm::vec3(0,500,0);
		float radius = 256.0f;
		PyramidDistanceFunction function(glm::vec3(0.0f), 1.0f);
		Transformation model2(glm::vec3(radius), center, 0,0,0);

		WrappedPyramid wrappedFunction = WrappedPyramid(&function, minSize, model2);
		solidSpace.add(wrappedFunction, model2, SimpleBrush(4), minSize, simplifier, &solidSpaceChangeHandler);
	}

	{
		std::cout << "\tsolidSpace.add(water)"<< std::endl;
		BoundingBox waterBox = mapBox;
		waterBox.setMaxY(0);
		OctreeDifferenceFunction function(&solidSpace, waterBox);
		WrappedOctreeDifference wrappedFunction = WrappedOctreeDifference(&function, minSize, model);
		wrappedFunction.cacheEnabled = true;
		liquidSpace.add(wrappedFunction, model, WaterBrush(0), minSize, simplifier, &liquidSpaceChangeHandler);
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
	HeightMapDistanceFunction function(heightMap);
	WrappedHeightMap wrappedFunction = WrappedHeightMap(&function, minSize, model);
	solidSpace.add(wrappedFunction, model, DerivativeLandBrush(), minSize, simplifier, &solidSpaceChangeHandler);

	BoundingBox waterBox = mapBox;
	waterBox.setMaxY(0);
	
	//exportOctree();
}

void Scene::save(std::string folderPath, Camera &camera) {
	OctreeFile saver1(&solidSpace, "solid", 9);
	OctreeFile saver2(&liquidSpace, "liquid", 9);
	saver1.save(folderPath, 4096);
	saver2.save(folderPath, 4096);
}

void Scene::load(std::string folderPath, Camera &camera) {
	OctreeFile loader1(&solidSpace, "solid", 9);
	OctreeFile loader2(&liquidSpace, "liquid", 9);
	loader1.load(folderPath, 4096);
	loader2.load(folderPath, 4096);
	//camera.position.x = loader1.getBox().getCenter().x;
	//camera.position.y = loader1.getBox().getMaxY();
	//camera.position.z = loader1.getBox().getCenter().z;
}

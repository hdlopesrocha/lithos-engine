
#include "tools.hpp"

Scene::Scene(Settings * settings):
    simplifier(0.99, 0.1, true)

 {
	this->settings = settings;
	solidSpace = new Octree(BoundingCube(glm::vec3(0,0,0), 30.0), glm::pow(2, 9));
	liquidSpace = new Octree(BoundingCube(glm::vec3(0,7,0), 30.0), glm::pow(2, 9));

	solidTrianglesCount = 0;
	liquidTrianglesCount = 0;

	solidInstancesVisible = 0;
	liquidInstancesVisible = 0;
	vegetationInstancesVisible = 0;


	solidBuilder = new MeshGeometryBuilder(&solidTrianglesCount, solidSpace);
	liquidBuilder = new MeshGeometryBuilder(&liquidTrianglesCount, liquidSpace);
	vegetationBuilder = new VegetationGeometryBuilder(solidSpace, new VegetationInstanceBuilderHandler(solidSpace, 0.1, 4));
	debugBuilder = new OctreeGeometryBuilder(new OctreeInstanceBuilderHandler());

	solidRenderer = new OctreeVisibilityChecker(&visibleSolidNodes);
	liquidRenderer = new OctreeVisibilityChecker(&visibleLiquidNodes);
	for(int i = 0 ; i < SHADOW_MATRIX_COUNT ; ++i) {
		shadowRenderer[i]= new OctreeVisibilityChecker(&visibleShadowNodes[i]);
	}
}

bool Scene::loadSpace(Octree* tree, OctreeNodeData& data, std::unordered_map<long, NodeInfo>* infos, GeometryBuilder* builder) {
	InstanceGeometry* loadable = builder->build(data);
	if (loadable == NULL) {
		// No geometry to load — erase entry if it exists
		infos->erase(data.node->id);
		return false;
	}

	// Try to insert a new NodeInfo with loadable
	auto [it, inserted] = infos->try_emplace(data.node->id, loadable);
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

bool Scene::processSpace() {
	// Set load counts per Processor
	solidInstancesVisible = 0;
	liquidInstancesVisible = 0;
	vegetationInstancesVisible = 0;
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

	int i =0;
	for(std::pair<glm::mat4, glm::vec3> pair :  lightProjection){
		setVisibleNodes(solidSpace, pair.first, pair.second, *shadowRenderer[i++]);
	}
}

void Scene::setVisibleNodes(Octree * tree, glm::mat4 viewProjection, glm::vec3 sortPosition, OctreeVisibilityChecker &checker) {
	checker.visibleNodes->clear();
	checker.sortPosition = sortPosition;
	checker.update(viewProjection);
	tree->iterateFlat(checker);	//here we get the visible nodes for that LOD + geometryLEvel
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
		
		if(drawableType == TYPE_INSTANCE_VEGETATION_DRAWABLE) {
			DrawableInstanceGeometry * drawable	= loadIfNeeded(&vegetationInfo, node->id);
			if(drawable != NULL) {
				float amount = glm::clamp( 1.0 - glm::length(cameraPosition -  drawable->center)/(float(settings->billboardRange)), 0.0, 1.0);
				if(amount > 0.8){
					amount = 1.0;
				}
				//std::cout << "Scene.draw() " << std::to_string(drawableType) << "|" << std::to_string(amount) << std::endl;
				drawable->draw(mode, amount, &vegetationInstancesVisible);
			}
		}
		else if(drawableType == TYPE_INSTANCE_SOLID_DRAWABLE) {
			DrawableInstanceGeometry * drawable	= loadIfNeeded(&solidInfo, node->id);
			if(drawable != NULL) {
				drawable->draw(mode, 1.0, &solidInstancesVisible);
			}
		}
		else if(drawableType == TYPE_INSTANCE_LIQUID_DRAWABLE) {
			DrawableInstanceGeometry * drawable	= loadIfNeeded(&liquidInfo, node->id);
			if(drawable != NULL) {
				drawable->draw(mode, 1.0, &liquidInstancesVisible);
			}
		}
		else if(drawableType == TYPE_INSTANCE_OCTREE_DRAWABLE) {
			DrawableInstanceGeometry * drawable	= loadIfNeeded(&debugInfo, node->id);
			if(drawable != NULL) {
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

	{
		BoundingBox box = BoundingBox(glm::vec3(1500,0,0),glm::vec3(1500+256,256,256));
		solidSpace->add(BoxContainmentHandler(box), BoxDistanceFunction(box), SimpleBrush(4), DirtyHandler(*this), 2.0, simplifier);
	}

	{
		BoundingBox box = BoundingBox(glm::vec3(2000,0,0),glm::vec3(2000+256,256,256));
		solidSpace->add(BoxContainmentHandler(box), BoxDistanceFunction(box), SimpleBrush(4), DirtyHandler(*this), minSize, simplifier);
	}

	{
		BoundingBox box = BoundingBox(glm::vec3(2500,0,0),glm::vec3(2500+256,256,256));
		solidSpace->add(BoxContainmentHandler(box), BoxDistanceFunction(box), SimpleBrush(4), DirtyHandler(*this), minSize*2, simplifier);
	}
	BoundingBox mapBox = BoundingBox(glm::vec3(-sizePerTile*tiles*0.5,-height*0.5,-sizePerTile*tiles*0.5), glm::vec3(sizePerTile*tiles*0.5,height*0.5,sizePerTile*tiles*0.5));
	camera.position.x = mapBox.getCenter().x;
	camera.position.y = mapBox.getMaxY();
	camera.position.z = mapBox.getCenter().z;

	GradientPerlinSurface heightFunction = GradientPerlinSurface(height, 1.0/(256.0f*sizePerTile), -64);
	CachedHeightMapSurface cache = CachedHeightMapSurface(heightFunction, mapBox, sizePerTile);
	HeightMap heightMap = HeightMap(cache, mapBox, sizePerTile);

	solidSpace->add(HeightMapContainmentHandler(heightMap), HeightMapDistanceFunction(heightMap), LandBrush(), DirtyHandler(*this), minSize, simplifier);

	{
		BoundingSphere sphere = BoundingSphere(glm::vec3(0,768,0),1024);
		solidSpace->del(SphereContainmentHandler(sphere), SphereDistanceFunction(sphere), SimpleBrush(14), DirtyHandler(*this), minSize, simplifier);
	}

	{
		BoundingBox box = BoundingBox(glm::vec3(1500,0,500),glm::vec3(1500+256,256,500+256));
		solidSpace->add(BoxContainmentHandler(box), BoxDistanceFunction(box), SimpleBrush(4), DirtyHandler(*this), 2.0, simplifier);
	}

	{
		BoundingBox box = BoundingBox(glm::vec3(2000,0,500),glm::vec3(2000+256,256,500+256));
		solidSpace->add(BoxContainmentHandler(box), BoxDistanceFunction(box), SimpleBrush(4), DirtyHandler(*this), minSize, simplifier);
	}

	{
		BoundingBox box = BoundingBox(glm::vec3(2500,0,500),glm::vec3(2500+256,256,500+256));
		solidSpace->add(BoxContainmentHandler(box), BoxDistanceFunction(box), SimpleBrush(4), DirtyHandler(*this), minSize*2, simplifier);
	}
	//solidSpace->add(BoxContainmentHandler(BoundingBox(glm::vec3(-20,-5,-20),glm::vec3(20,10,20)),SimpleBrush(8)), 1.0);
	//solidSpace->del(BoxContainmentHandler(BoundingBox(glm::vec3(-17,-4,-17),glm::vec3(17,12,17)),SimpleBrush(6)), 1.0);
	//liquidSpace->del(BoxContainmentHandler(BoundingBox(glm::vec3(-18,-5,-18),glm::vec3(18,12,18)),SimpleBrush(0)), 1.0);
	
	{
		BoundingSphere sphere = BoundingSphere(glm::vec3(0,512,0),128);
		solidSpace->add(SphereContainmentHandler(sphere), SphereDistanceFunction(sphere), SimpleBrush(4), DirtyHandler(*this), 8.0, simplifier);
	}
	//solidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(-11,61,11),10), SimpleBrush(5)), 1.0);
	//solidSpace->del(SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),10), SimpleBrush(4)), 1.0);
	//solidSpace->del(SphereContainmentHandler(BoundingSphere(glm::vec3(4,54,-4),8), SimpleBrush(1)), 1.0);

	//liquidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),4), SimpleBrush(0)));
	BoundingBox waterBox = mapBox;
	waterBox.setMaxY(0);
//	liquidSpace->add(OctreeContainmentHandler(solidSpace, waterBox, WaterBrush(0)), DirtyHandler(*this), minSize, simplifier);
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

	HeightMapTif heightFunction = HeightMapTif(filename, mapBox, sizePerTile,1.0f, -320.0f);
    CachedHeightMapSurface cache = CachedHeightMapSurface(heightFunction, mapBox, sizePerTile);
	HeightMap heightMap = HeightMap(cache, mapBox, sizePerTile);

	solidSpace->add(HeightMapContainmentHandler(heightMap), HeightMapDistanceFunction(heightMap), DerivativeLandBrush(), DirtyHandler(*this), minSize, simplifier);

	BoundingBox waterBox = mapBox;
	waterBox.setMaxY(0);
	
	//liquidSpace->add(OctreeContainmentHandler(solidSpace, waterBox, WaterBrush(0)), DirtyHandler(*this), minSize, simplifier);
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

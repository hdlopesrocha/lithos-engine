
#include "math/math.hpp"
#include "tools/tools.hpp"
#include "gl/gl.hpp"

class Scene {

    public: 
    	Octree * solidSpace = new Octree(BoundingCube(glm::vec3(0,0,0), 2.0));
	    Octree * liquidSpace = new Octree(BoundingCube(glm::vec3(0,20,0), 2.0));



		long solidInstancesCount = 0;
		long liquidInstancesCount = 0;
		long vegetationInstancesCount = 0;

		long solidInstancesVisible = 0;
		long liquidInstancesVisible = 0;
		long vegetationInstancesVisible = 0;
		
		std::vector<IteratorData> visibleSolidNodes;
		std::vector<IteratorData> visibleLiquidNodes;
		std::vector<IteratorData> visibleShadowNodes[SHADOW_MATRIX_COUNT];
		Settings * settings;
		int geometryLevel = 5;

		GeometryBuilder * vegetationBuilder = new VegetationGeometryBuilder(TYPE_INSTANCE_VEGETATION_DRAWABLE, &vegetationInstancesCount, solidSpace);
		GeometryBuilder * meshBuilder = new MeshGeometryBuilder(TYPE_INSTANCE_SOLID_DRAWABLE, &solidInstancesCount, solidSpace, 0.9, 0.2, true, geometryLevel);
		GeometryBuilder * liquidMeshBuilder = new MeshGeometryBuilder(TYPE_INSTANCE_LIQUID_DRAWABLE, &liquidInstancesCount, liquidSpace, 0.9, 0.2, true, geometryLevel);

		OctreeProcessor solidProcessor = OctreeProcessor(solidSpace , true, meshBuilder);
		OctreeProcessor liquidProcessor = OctreeProcessor(liquidSpace, true, liquidMeshBuilder);
		OctreeProcessor vegetationProcessor = OctreeProcessor(solidSpace, true, vegetationBuilder);

		OctreeVisibilityChecker solidRenderer = OctreeVisibilityChecker(solidSpace, &visibleSolidNodes);
		OctreeVisibilityChecker liquidRenderer = OctreeVisibilityChecker(liquidSpace, &visibleLiquidNodes);
		OctreeVisibilityChecker shadowRenderer[SHADOW_MATRIX_COUNT] = {
			OctreeVisibilityChecker(solidSpace, &visibleShadowNodes[0]),
			OctreeVisibilityChecker(solidSpace, &visibleShadowNodes[1]),
			OctreeVisibilityChecker(solidSpace, &visibleShadowNodes[2])
		};

    void setup(Settings * settings) {
		this->settings = settings;
    }



	void processSpace() {
		// Set load counts per Processor

		solidInstancesVisible = 0;
		liquidInstancesVisible = 0;
		vegetationInstancesVisible = 0;
		
		for(int i =0 ; i < SHADOW_MATRIX_COUNT ; ++i){
			std::vector<IteratorData> &vec = visibleShadowNodes[i];
			for(IteratorData &data : vec) {
				solidProcessor.loadCount = 1;
				solidProcessor.before(data.level,data.height,data.lod, data.node, data.cube, NULL);
				vegetationProcessor.loadCount = 1;
				vegetationProcessor.before(data.level,data.height, data.lod, data.node, data.cube, NULL);
			}
		}

		for(IteratorData &data : visibleSolidNodes){
			solidProcessor.loadCount = 1;
			solidProcessor.before(data.level,data.height, data.lod, data.node, data.cube, NULL);
			vegetationProcessor.loadCount = 1;
			vegetationProcessor.before(data.level,data.height, data.lod, data.node, data.cube, NULL);
		}

		for(IteratorData &data : visibleLiquidNodes){
			liquidProcessor.loadCount = 1;
			liquidProcessor.before(data.level,data.height,data.lod, data.node, data.cube, NULL);
		}
	}

	void setVisibility(glm::mat4 viewProjection, std::vector<std::pair<glm::mat4, glm::vec3>> lightProjection ,Camera &camera) {
		setVisibleNodes(viewProjection, camera.position, solidRenderer);
		setVisibleNodes(viewProjection, camera.position, liquidRenderer);

		int i =0;
		for(std::pair<glm::mat4, glm::vec3> pair :  lightProjection){
			setVisibleNodes(pair.first, pair.second, shadowRenderer[i++]);
		}
	}

	void setVisibleNodes(glm::mat4 viewProjection, glm::vec3 sortPosition, OctreeVisibilityChecker &checker) {
		checker.visibleNodes->clear();
		checker.sortPosition = sortPosition;
		
		checker.update(viewProjection);
		checker.tree->iterateFlat(checker, geometryLevel);	//here we get the visible nodes for that LOD + geometryLEvel
	}


	void draw (uint drawableType, int mode, glm::vec3 cameraPosition, const std::vector<IteratorData> &list) {
		//std::cout << "Scene.draw() " << std::to_string(drawableType) << "|" << std::to_string(list.size()) << std::endl;
		for(const IteratorData &data : list) {
			OctreeNode * node = data.node;
			NodeInfo * info = node->getNodeInfo(drawableType);
			
			if(info != NULL){
				if(info->temp != NULL) {
					PreLoadedGeometry * pre = (PreLoadedGeometry *) info->temp;
					std::vector<Vertex> &vertices = pre->geometry->vertices;
					
					DrawableInstanceGeometry * geo = new DrawableInstanceGeometry(info->type, pre->geometry, &pre->instances, pre->center);
					info->data = geo;
					info->temp = NULL;
					if(info->type != TYPE_INSTANCE_VEGETATION_DRAWABLE){
						delete pre->geometry;
					} 
					delete pre;
				}
				
				DrawableInstanceGeometry * drawable = (DrawableInstanceGeometry*) info->data;
			
				if(drawableType == TYPE_INSTANCE_VEGETATION_DRAWABLE) {
					float amount = glm::clamp( 1.0 - glm::length(cameraPosition -  drawable->center)/(float(settings->billboardRange)), 0.0, 1.0);
					if(amount > 0.8){
						amount = 1.0;
					}

					//std::cout << "Scene.TYPE_INSTANCE_VEGETATION_DRAWABLE() " << std::to_string(vegetationInstancesVisible) << std::endl;
					drawable->draw(mode, amount);
					vegetationInstancesVisible += long(ceil(drawable->instancesCount*amount));
				}else if(drawableType == TYPE_INSTANCE_SOLID_DRAWABLE) {
					//std::cout << "Scene.TYPE_INSTANCE_SOLID_DRAWABLE() " << std::to_string(solidInstancesVisible) << std::endl;
					drawable->draw(mode, 1.0);
					solidInstancesVisible += drawable->instancesCount;
				}
				else if(drawableType == TYPE_INSTANCE_LIQUID_DRAWABLE) {
					//std::cout << "Scene.TYPE_INSTANCE_LIQUID_DRAWABLE() " << std::to_string(liquidInstancesVisible) << std::endl;
					drawable->draw(mode, 1.0);
					liquidInstancesVisible += drawable->instancesCount;
				}
			
			}
		}
	}

	void drawVegetation(glm::vec3 cameraPosition, const std::vector<IteratorData> &list) {
		glDisable(GL_CULL_FACE);
		draw(TYPE_INSTANCE_VEGETATION_DRAWABLE, GL_PATCHES, cameraPosition, list);
		glEnable(GL_CULL_FACE);
	}

	void draw3dSolid(glm::vec3 cameraPosition, const std::vector<IteratorData> &list) {
		draw(TYPE_INSTANCE_SOLID_DRAWABLE, GL_PATCHES, cameraPosition, list);
	}

	void draw3dLiquid(glm::vec3 cameraPosition, const std::vector<IteratorData> &list) {
		draw(TYPE_INSTANCE_LIQUID_DRAWABLE, GL_PATCHES, cameraPosition, list);
	}

	void create() {

		BoundingBox mapBox(glm::vec3(-200,-60,-200), glm::vec3(200,50,200));
		
		solidSpace->add(HeightMapContainmentHandler(
			HeightMap(
				CachedHeightMapSurface(
					GradientPerlinSurface(100, 1.0f/128.0f, 0), 
					mapBox, solidSpace->minSize
				), mapBox, solidSpace->minSize
			), LandBrush()
		));
		solidSpace->add(BoxContainmentHandler(BoundingBox(glm::vec3(-10,6,-10),glm::vec3(34,50,34)),SimpleBrush(8)));
		solidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(0,50,0),20), SimpleBrush(6)));
		solidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(-11,61,11),10), SimpleBrush(5)));
		solidSpace->del(SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),10), SimpleBrush(4)));
		solidSpace->del(SphereContainmentHandler(BoundingSphere(glm::vec3(4,54,-4),8), SimpleBrush(1)));

		BoundingBox waterBox(glm::vec3(-200,-60,-200), glm::vec3(200,3,200));
		liquidSpace->add(OctreeContainmentHandler(solidSpace, waterBox, WaterBrush(0)));
		liquidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),4), SimpleBrush(0)));
	}


	void save() {
		OctreeFile saver1(solidSpace, "solid", 6);
		OctreeFile saver2(liquidSpace, "liquid", 6);
		saver1.save();
		saver2.save();
	}

	void load() {
		OctreeFile loader1(solidSpace, "solid", 6);
		OctreeFile loader2(liquidSpace, "liquid", 6);
		loader1.load();
		loader2.load();
	}
};
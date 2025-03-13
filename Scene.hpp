
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
		long shadowInstancesCount = 0;
		long solidInstancesVisible = 0;
		long liquidInstancesVisible = 0;
		long vegetationInstancesVisible = 0;
		std::vector<IteratorData> visibleSolidNodes;
		std::vector<IteratorData> visibleLiquidNodes;
		std::vector<IteratorData> visibleShadowNodes[SHADOW_MATRIX_COUNT];
		Settings * settings;
		OctreeProcessor solidProcessor = OctreeProcessor(solidSpace, &solidInstancesCount, TYPE_INSTANCE_SOLID_DRAWABLE , 5, 0.9, 0.2, true, true, 5);
		OctreeProcessor liquidProcessor = OctreeProcessor(liquidSpace, &liquidInstancesCount, TYPE_INSTANCE_LIQUID_DRAWABLE, 5, 0.9, 0.2, true, true, 5);
		OctreeProcessor vegetationProcessor = OctreeProcessor(solidSpace, &vegetationInstancesCount,  TYPE_INSTANCE_VEGETATION_DRAWABLE, 5, 0.9, 0.2, true, true, 5);

		OctreeVisibilityChecker solidRenderer = OctreeVisibilityChecker(solidSpace, 5, &visibleSolidNodes);
		OctreeVisibilityChecker liquidRenderer = OctreeVisibilityChecker(liquidSpace, 5, &visibleLiquidNodes);;
		OctreeVisibilityChecker shadowRenderer[SHADOW_MATRIX_COUNT] = {
			OctreeVisibilityChecker(solidSpace, 5, &visibleShadowNodes[0]),
			OctreeVisibilityChecker(solidSpace, 5, &visibleShadowNodes[1]),
			OctreeVisibilityChecker(solidSpace, 5, &visibleShadowNodes[2])
		};

    void setup(Settings * settings) {
		this->settings = settings;
    }



	void processSpace() {
		solidProcessor.loadCount = 1;
		liquidProcessor.loadCount = 1;
		vegetationProcessor.loadCount = 1;
		
		solidInstancesVisible = 0;
		liquidInstancesVisible = 0;
		vegetationInstancesVisible = 0;
		
		for(int i =0 ; i < SHADOW_MATRIX_COUNT ; ++i){
			std::vector<IteratorData> &vec = visibleShadowNodes[i];
			for(IteratorData &data : vec) {
				solidProcessor.before(data.level,data.height, data.node, data.cube, NULL);
				vegetationProcessor.before(data.level,data.height, data.node, data.cube, NULL);
			}
		}

		for(IteratorData &data : visibleSolidNodes){
			solidProcessor.before(data.level,data.height, data.node, data.cube, NULL);
			vegetationProcessor.before(data.level,data.height, data.node, data.cube, NULL);
		}

		for(IteratorData &data : visibleLiquidNodes){
			liquidProcessor.before(data.level,data.height, data.node, data.cube, NULL);
		}
	}

	void setVisibility(glm::mat4 viewProjection, std::vector<std::pair<glm::mat4, glm::vec3>> lightProjection ,Camera &camera) {
		setVisibleNodes(viewProjection, camera.position, solidRenderer);
		setVisibleNodes(viewProjection, camera.position, liquidRenderer);
		for(int i=0 ; i< SHADOW_MATRIX_COUNT; ++i){
			setVisibleNodes(lightProjection[i].first, lightProjection[i].second, shadowRenderer[i]);
		}
	}

	void setVisibleNodes(glm::mat4 viewProjection, glm::vec3 sortPosition, OctreeVisibilityChecker &checker) {
		checker.visibleNodes->clear();
		checker.sortPosition = sortPosition;
		
		checker.update(viewProjection);
		checker.tree->iterateFlat(checker);
	}


	void draw (uint drawableType, int mode, glm::vec3 cameraPosition, const std::vector<IteratorData> &list) {
		for(const IteratorData &data : list) {
			OctreeNode * node = data.node;
			for(NodeInfo &info : node->info ) {
			
				if(info.type & drawableType){
					if(info.temp != NULL) {
						PreLoadedGeometry * pre = (PreLoadedGeometry *) info.temp;
						std::vector<Vertex> &vertices = pre->geometry->vertices;
						
						DrawableInstanceGeometry * geo = new DrawableInstanceGeometry(pre->geometry, &pre->instances);
						info.data = geo;
						info.temp = NULL;
						delete pre->geometry;
						delete pre;
					}
					
					DrawableInstanceGeometry * drawable = (DrawableInstanceGeometry*) info.data;
				
					float amount = 1.0;
					if(drawableType & TYPE_INSTANCE_VEGETATION_DRAWABLE) {
						amount = glm::clamp( 1.0 - glm::length(cameraPosition -  drawable->center)/(float(settings->billboardRange)), 0.0, 1.0);
						if(amount > 0.8){
							amount = 1.0;
						}
					}
					drawable->draw(mode, amount);
					
					if(drawableType & TYPE_INSTANCE_SOLID_DRAWABLE) {
						solidInstancesVisible += drawable->instancesCount;
					}
					else if(drawableType & TYPE_INSTANCE_LIQUID_DRAWABLE) {
						liquidInstancesVisible += drawable->instancesCount;
					}
					else if(drawableType & TYPE_INSTANCE_VEGETATION_DRAWABLE) {
						vegetationInstancesVisible += long(ceil(drawable->instancesCount*amount));
					} 
				}
			}
		}
	}


	void drawBillboards(glm::vec3 cameraPosition, const std::vector<IteratorData> &list) {
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
		solidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),4), SimpleBrush(0)));

		BoundingBox waterBox(glm::vec3(-200,-60,-200), glm::vec3(200,3,200));
		liquidSpace->add(OctreeContainmentHandler(solidSpace, waterBox, WaterBrush(0)));
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

#include "math/math.hpp"
#include "tools/tools.hpp"
#include "gl/gl.hpp"

class Scene {

    public: 
    	Octree * solidSpace;
	    Octree * liquidSpace;
		OctreeVisibilityChecker * solidRenderer;
		OctreeVisibilityChecker * liquidRenderer;
		OctreeVisibilityChecker * shadowRenderer[SHADOW_MATRIX_COUNT];
		OctreeProcessor * solidProcessor;
		OctreeProcessor * liquidProcessor;
		OctreeProcessor * vegetationProcessor;
		OctreeProcessor * shadowProcessor;

		int solidInstancesCount = 0;
		int liquidInstancesCount = 0;
		int vegetationInstancesCount = 0;
		int shadowInstancesCount = 0;

		int solidInstancesVisible = 0;
		int liquidInstancesVisible = 0;
		int vegetationInstancesVisible = 0;
		std::vector<IteratorData> visibleSolidNodes;
		std::vector<IteratorData> visibleLiquidNodes;
		std::vector<IteratorData> visibleShadowNodes[SHADOW_MATRIX_COUNT];



    void setup() {
		solidSpace = new Octree(BoundingCube(glm::vec3(0,0,0), 2.0));
		liquidSpace = new Octree(BoundingCube(glm::vec3(0,20,0), 2.0));
  
		solidProcessor = new OctreeProcessor(solidSpace, &solidInstancesCount, TYPE_INSTANCE_SOLID_DRAWABLE, 5, 0.9, 0.2, true, true, 5);
		liquidProcessor = new OctreeProcessor(liquidSpace, &liquidInstancesCount, TYPE_INSTANCE_LIQUID_DRAWABLE, 5, 0.9, 0.2, true, true, 5);
		vegetationProcessor = new OctreeProcessor(solidSpace, &vegetationInstancesCount, TYPE_INSTANCE_VEGETATION_DRAWABLE, 5, 0.9, 0.2, true, true, 5);
		shadowProcessor = new OctreeProcessor(solidSpace, &shadowInstancesCount, TYPE_INSTANCE_SHADOW_DRAWABLE, 5, 0.9, 0.2, true, true, 5);

		solidRenderer = new OctreeVisibilityChecker(solidSpace, 5, &visibleSolidNodes);
		liquidRenderer = new OctreeVisibilityChecker(liquidSpace, 5, &visibleLiquidNodes);
		shadowRenderer[0] = new OctreeVisibilityChecker(solidSpace, 5, &visibleShadowNodes[0]);
		shadowRenderer[1] = new OctreeVisibilityChecker(solidSpace, 5, &visibleShadowNodes[1]);
		shadowRenderer[2] = new OctreeVisibilityChecker(solidSpace, 5, &visibleShadowNodes[2]);

    }

	void draw (int drawableType, int mode, Settings * settings, glm::vec3 cameraPosition, std::vector<IteratorData> * list) {
		for(const IteratorData &data : *list) {
			OctreeNode * node = data.node;
			for(NodeInfo &info : node->info ) {
			
				if(info.type == drawableType){
					if(info.temp != NULL) {
						PreLoadedGeometry * pre = (PreLoadedGeometry *) info.temp;
						DrawableInstanceGeometry * geo = new DrawableInstanceGeometry(pre->geometry, &pre->instances, pre->center);
						info.data = geo;
						info.temp = NULL;
						delete pre->geometry;
						delete pre;
					}
					
					DrawableInstanceGeometry * drawable = (DrawableInstanceGeometry*) info.data;
					//std::cout << "Current LOD " << std::to_string(currentLod) << " | " << std::to_string(selectedLod) << std::endl;

					//std::cout << "Draw " << std::to_string(drawable->instancesCount) << " | " << std::to_string(drawableType) << std::endl;
					if(drawableType == TYPE_INSTANCE_VEGETATION_DRAWABLE) {
						float range = settings->billboardRange;

						float amount = Math::clamp(1.0- glm::distance2(cameraPosition, drawable->center)/(range * range), 0.0f, 1.0f); 
						if(amount > 0.8) {
							amount = 1.0;
						}
						
						drawable->draw(mode, amount);
						vegetationInstancesVisible += drawable->instancesCount*amount;
					} else {
						drawable->draw(mode);
						if(drawableType == TYPE_INSTANCE_SOLID_DRAWABLE) {
							solidInstancesVisible += drawable->instancesCount;
						}else if(drawableType == TYPE_INSTANCE_LIQUID_DRAWABLE) {
							liquidInstancesVisible += drawable->instancesCount;
						}
					}
				
				}
			}
		}
	}


	void processSpace() {
		solidProcessor->loadCount = 1;
		liquidProcessor->loadCount = 1;
		vegetationProcessor->loadCount = 1;
		shadowProcessor->loadCount = 1;
		solidInstancesVisible = 0;
		liquidInstancesVisible = 0;
		vegetationInstancesVisible = 0;

		for(int i =0 ; i < 3 ; ++i){
			for(IteratorData &data : visibleShadowNodes[i]) {
				shadowProcessor->before(data.level,data.height, data.node, data.cube, NULL);
			}
		}


		for(IteratorData &data : visibleSolidNodes){
			solidProcessor->before(data.level,data.height, data.node, data.cube, NULL);
			vegetationProcessor->before(data.level,data.height, data.node, data.cube, NULL);
		}
		for(IteratorData &data : visibleLiquidNodes){
			liquidProcessor->before(data.level,data.height, data.node, data.cube, NULL);
		}
	}


	void setVisibility(glm::mat4 viewProjection, std::vector<glm::mat4> shadowMatrices ,Camera * camera, DirectionalLight * light, Settings * settings) {
		float far = 512.0;
		setVisibleNodes(viewProjection, camera->position, *solidRenderer);
		setVisibleNodes(viewProjection, camera->position, *liquidRenderer);
		if(settings->shadowEnabled) {
			glm::vec3 fakeLightPosition = camera->position - light->direction * far *0.5f;
			for(int i=0 ; i < shadowMatrices.size() ; ++i) {
				setVisibleNodes(shadowMatrices[i], fakeLightPosition, *shadowRenderer[i]);
			}
		}
	}

	void setVisibleNodes(glm::mat4 viewProjection, glm::vec3 sortPosition, OctreeVisibilityChecker &checker) {
		checker.visibleNodes->clear();
		checker.sortPosition = sortPosition;
		checker.update(viewProjection);
		checker.tree->iterateFlat(checker);
	}


	void drawBillboards(glm::vec3 cameraPosition, Settings * settings, std::vector<IteratorData> * list) {
		glDisable(GL_CULL_FACE);
		draw(TYPE_INSTANCE_VEGETATION_DRAWABLE, GL_PATCHES, settings, cameraPosition, list);
		glEnable(GL_CULL_FACE);
	}

	void draw3dSolid(glm::vec3 cameraPosition, Settings * settings, std::vector<IteratorData> * list) {
		draw(TYPE_INSTANCE_SOLID_DRAWABLE, GL_PATCHES, settings, cameraPosition, list);
	}

	void draw3dLiquid(glm::vec3 cameraPosition, Settings * settings, std::vector<IteratorData> * list) {
		draw(TYPE_INSTANCE_LIQUID_DRAWABLE, GL_PATCHES, settings, cameraPosition, list);
	}

	void create(std::vector<Brush*>  brushes) {

		BoundingBox mapBox(glm::vec3(-200,-60,-200), glm::vec3(200,50,200));
		
		solidSpace->add(HeightMapContainmentHandler(
			HeightMap(
				CachedHeightMapSurface(
					GradientPerlinSurface(100, 1.0f/128.0f, 0), 
					mapBox, solidSpace->minSize
				), mapBox, solidSpace->minSize
			), LandBrush(brushes)
		));
		solidSpace->add(BoxContainmentHandler(BoundingBox(glm::vec3(-10,6,-10),glm::vec3(34,50,34)),SimpleBrush(brushes[8])));
		solidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(0,50,0),20), SimpleBrush(brushes[6])));
		solidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(-11,61,11),10), SimpleBrush(brushes[5])));
		solidSpace->del(SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),10), SimpleBrush(brushes[4])));
		solidSpace->del(SphereContainmentHandler(BoundingSphere(glm::vec3(4,54,-4),8), SimpleBrush(brushes[1])));
		solidSpace->add(SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),4), SimpleBrush(brushes[0])));

		BoundingBox waterBox(glm::vec3(-200,-60,-200), glm::vec3(200,3,200));
		liquidSpace->add(OctreeContainmentHandler(solidSpace, waterBox, WaterBrush(brushes[0])));
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
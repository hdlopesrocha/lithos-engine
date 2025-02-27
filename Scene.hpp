
#include "math/math.hpp"
#include "tools/tools.hpp"
#include "gl/gl.hpp"

class Scene {

    public: 
    	Octree * solidSpace;
	    Octree * liquidSpace;
		OctreeInstanceRenderer * solidRenderer;
		OctreeInstanceRenderer * shadowRenderer;
		OctreeInstanceRenderer * liquidRenderer;
		OctreeInstanceRenderer * billboardRenderer;
		OctreeProcessor * solidProcessor;
		OctreeProcessor * shadowProcessor;
		OctreeProcessor * liquidProcessor;
		OctreeProcessor * vegetationProcessor;

		int solidInstancesCount = 0;
		int liquidInstancesCount = 0;
		int shadowInstancesCount = 0;
		int vegetationInstancesCount = 0;

		int solidInstancesVisible = 0;
		int liquidInstancesVisible = 0;
		int shadowInstancesVisible = 0;
		int vegetationInstancesVisible = 0;

    void setup(Settings * settings) {

		solidSpace = new Octree(BoundingCube(glm::vec3(0,0,0), 2.0));
		liquidSpace = new Octree(BoundingCube(glm::vec3(0,20,0), 2.0));
  
		solidProcessor = new OctreeProcessor(solidSpace, &solidInstancesCount, TYPE_INSTANCE_SOLID_DRAWABLE, 5, 0.9, 0.2, true, true, 5);
		liquidProcessor = new OctreeProcessor(liquidSpace, &liquidInstancesCount, TYPE_INSTANCE_LIQUID_DRAWABLE, 5, 0.9, 0.2, true, true, 5);
		shadowProcessor = new OctreeProcessor(solidSpace, &shadowInstancesCount, TYPE_INSTANCE_SHADOW_DRAWABLE, 6, 0.1, 4.0, false, true, 6);
		vegetationProcessor = new OctreeProcessor(solidSpace, &vegetationInstancesCount, TYPE_INSTANCE_VEGETATION_DRAWABLE, 5, 0.9, 0.2, false, true, 5);

		solidRenderer = new OctreeInstanceRenderer(solidSpace, &solidInstancesVisible, GL_PATCHES, TYPE_INSTANCE_SOLID_DRAWABLE, 5,settings);
		liquidRenderer = new OctreeInstanceRenderer(liquidSpace, &liquidInstancesVisible, GL_PATCHES, TYPE_INSTANCE_LIQUID_DRAWABLE, 5,settings);
		shadowRenderer = new OctreeInstanceRenderer(solidSpace, &shadowInstancesVisible, GL_PATCHES, TYPE_INSTANCE_SHADOW_DRAWABLE, 6,settings);
		billboardRenderer = new OctreeInstanceRenderer(solidSpace, &vegetationInstancesVisible, GL_PATCHES, TYPE_INSTANCE_VEGETATION_DRAWABLE, 5,settings);

    }

	void processSpace() {
		solidSpace->iterate(solidProcessor);
		liquidSpace->iterate(liquidProcessor);
		solidSpace->iterate(shadowProcessor);
		solidSpace->iterate(vegetationProcessor);
	}


	void update3d(glm::mat4 vp, Camera * camera) {
		billboardRenderer->cameraPosition = camera->position;
		solidRenderer->cameraPosition = camera->position;
		liquidRenderer->cameraPosition = camera->position;
	
		solidProcessor->loaded = 0;
		solidProcessor->update(vp);

		liquidProcessor->loaded = 0;
		liquidProcessor->update(vp);

		vegetationProcessor->loaded = 0;
		vegetationProcessor->update(vp);



		processSpace();
		solidInstancesVisible = 0;
		liquidInstancesVisible = 0;
		shadowInstancesVisible = 0;
		vegetationInstancesVisible = 0;


		shadowProcessor->loaded = 0;

	}

	void updateShadow(glm::mat4 lp, Camera * camera, DirectionalLight * light) {
		shadowRenderer->cameraPosition = camera->position -light->direction*512.0f;
		shadowProcessor->update(lp);
	}

	void drawBillboards(glm::mat4 viewProjection) {
		billboardRenderer->update(viewProjection);
		glDisable(GL_CULL_FACE);
		solidSpace->iterate(billboardRenderer);
		glEnable(GL_CULL_FACE);
	}

	void draw3dSolid(glm::mat4 viewProjection) {
		solidRenderer->update(viewProjection);
		solidSpace->iterate(solidRenderer);
	}

	void draw3dLiquid(glm::mat4 viewProjection) {
		liquidRenderer->update(viewProjection);
		liquidSpace->iterate(liquidRenderer);
	}

	void draw3dShadow(glm::mat4 viewProjection) {
		shadowRenderer->update(viewProjection);
		solidSpace->iterate(shadowRenderer);
	}


	void create(std::vector<Texture*> textures, std::vector<Brush*>  brushes) {

		BoundingBox mapBox(glm::vec3(-200,-60,-200), glm::vec3(200,50,200));
		HeightFunction * function = new GradientPerlinSurface(100, 1.0f/128.0f, 0);
		CachedHeightMapSurface * surface = new CachedHeightMapSurface(function, mapBox, solidSpace->minSize);
		HeightMap map(surface, mapBox.getMin(),mapBox.getMax(), solidSpace->minSize);

		solidSpace->add(new HeightMapContainmentHandler(&map, new LandBrush(brushes)));
		//tree->del(new SphereContainmentHandler(BoundingSphere(glm::vec3(00,-30,0),50), brushes[7]));
		solidSpace->add(new SphereContainmentHandler(BoundingSphere(glm::vec3(0,50,0),20), new SimpleBrush(brushes[6])));
		solidSpace->add(new SphereContainmentHandler(BoundingSphere(glm::vec3(-11,61,11),10), new SimpleBrush(brushes[5])));
		solidSpace->del(new SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),10), new SimpleBrush(brushes[4])));
		solidSpace->add(new BoxContainmentHandler(BoundingBox(glm::vec3(-10,6,-10),glm::vec3(34,50,34)),new SimpleBrush(brushes[8])));
		solidSpace->del(new SphereContainmentHandler(BoundingSphere(glm::vec3(4,54,-4),8), new SimpleBrush(brushes[1])));
		solidSpace->add(new SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),4), new SimpleBrush(brushes[0])));

		BoundingBox waterBox(glm::vec3(-200,-60,-200), glm::vec3(200,3,200));
		//liquidSpace->add(new OctreeContainmentHandler(solidSpace, waterBox, new SimpleBrush(brushes[6])));
		//BoundingBox waterBox(glm::vec3(50,50,0), glm::vec3(70,70,20));
		liquidSpace->add(new BoxContainmentHandler(waterBox, new WaterBrush(brushes[0])));
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
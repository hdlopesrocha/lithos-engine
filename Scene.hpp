
#include "math/math.hpp"
#include "tools/tools.hpp"
#include <thread>

#define TYPE_SOLID_DRAWABLE 1
#define TYPE_SHADOW_DRAWABLE 2
#define TYPE_LIQUID_DRAWABLE 3




class Scene {

    public: 
    	Octree * solidSpace;
	    Octree * liquidSpace;
		OctreeRenderer * solidRenderer;
		OctreeRenderer * shadowRenderer;
		OctreeRenderer * liquidRenderer;
		OctreeProcessor * solidProcessor;
		OctreeProcessor * shadowProcessor;
		OctreeProcessor * liquidProcessor;
		Vegetation3d * vegetation;

		Camera camera;
		DirectionalLight light;

		int solidTrianglesCount = 0;
		int liquidTrianglesCount = 0;
		int shadowTrianglesCount = 0;

    void setup(	) {

        camera.quaternion =   glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1))
   	    					* glm::angleAxis(glm::radians(145.0f), glm::vec3(1, 0, 0))
   	    					* glm::angleAxis(glm::radians(135.0f), glm::vec3(0, 1, 0));  
		camera.position = glm::vec3(48,48,48);
        light.direction = glm::normalize(glm::vec3(-1.0,-1.0,-1.0));


		solidSpace = new Octree(BoundingCube(glm::vec3(0,0,0), 2.0));
		liquidSpace = new Octree(BoundingCube(glm::vec3(0,20,0), 2.0));
  
		solidRenderer = new OctreeRenderer(solidSpace, TYPE_SOLID_DRAWABLE, 5);
		liquidRenderer = new OctreeRenderer(liquidSpace, TYPE_LIQUID_DRAWABLE, 5);
		shadowRenderer = new OctreeRenderer(solidSpace, TYPE_SHADOW_DRAWABLE, 6);


		solidProcessor = new OctreeProcessor(solidSpace, &solidTrianglesCount, TYPE_SOLID_DRAWABLE, 5, 0.9, 0.2, true);
		liquidProcessor = new OctreeProcessor(liquidSpace, &liquidTrianglesCount, TYPE_LIQUID_DRAWABLE, 5, 0.9, 0.2, true);
		shadowProcessor = new OctreeProcessor(solidSpace, &shadowTrianglesCount, TYPE_SHADOW_DRAWABLE, 6, 0.1, 4.0, false);

		vegetation = new Vegetation3d();
    }

	void draw3dShadow() {
		solidSpace->iterate(shadowRenderer);
	}

bool isProcessing = false;
	void processSpace() {
		solidSpace->iterate(solidProcessor);
		liquidSpace->iterate(liquidProcessor);
		solidSpace->iterate(shadowProcessor);
		isProcessing = false;
	}


	void update3d(glm::mat4 mvp, glm::mat4 mlp) {
		solidProcessor->loaded = 0;
		liquidProcessor->loaded = 0;
		shadowProcessor->loaded = 0;

		solidRenderer->cameraPosition = camera.position;
		liquidRenderer->cameraPosition = camera.position;
		shadowRenderer->cameraPosition = camera.position -light.direction*512.0f;
	
		solidRenderer->update(mvp);
		solidProcessor->update(mvp);
		solidRenderer->mode = GL_PATCHES;

		liquidRenderer->update(mvp);
		liquidProcessor->update(mvp);
		liquidRenderer->mode = GL_PATCHES;

		shadowRenderer->update(mlp);
		shadowProcessor->update(mlp);
		shadowRenderer->mode = GL_TRIANGLES;

		if(!isProcessing) {
			isProcessing = true;
			std::thread t(&Scene::processSpace, this); 
			t.join();
		}
	}

	void drawVegetation() {
		glDisable(GL_CULL_FACE); // Enable face culling
		vegetation->drawable->draw(GL_TRIANGLES);
		glEnable(GL_CULL_FACE); // Enable face culling
	}

	void draw3dSolid() {
		solidSpace->iterate(solidRenderer);
	}

	void draw3dLiquid() {
		liquidSpace->iterate(liquidRenderer);
	}

	void create(std::vector<Texture*> textures, std::vector<Brush*>  brushes) {

		BoundingBox mapBox(glm::vec3(-100,-60,-100), glm::vec3(100,50,100));
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
		solidSpace->add(new SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),4), new SimpleBrush(brushes[16])));

		BoundingBox waterBox(glm::vec3(-100,-60,-100), glm::vec3(100,3,100));
		//liquidSpace->add(new OctreeContainmentHandler(solidSpace, waterBox, new SimpleBrush(brushes[6])));
		//BoundingBox waterBox(glm::vec3(50,50,0), glm::vec3(70,70,20));
		liquidSpace->add(new BoxContainmentHandler(waterBox, new WaterBrush(brushes[16])));
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
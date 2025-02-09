
#include "math/math.hpp"
#include "tools/tools.hpp"

#define TYPE_SOLID_DRAWABLE 1
#define TYPE_SHADOW_DRAWABLE 2
#define TYPE_LIQUID_DRAWABLE 3


glm::mat4 getCanonicalMVP(glm::mat4 m) {
	return glm::translate(glm::mat4(1.0f), glm::vec3(0.5)) 
					* glm::scale(glm::mat4(1.0f), glm::vec3(0.5)) 
					* m;
}

class Scene {

    public: 
    	Octree * solidSpace;
	    Octree * liquidSpace;
		OctreeRenderer * solidRenderer;
		OctreeRenderer * shadowRenderer;
		OctreeRenderer * liquidRenderer;
		Camera camera;
		DirectionalLight light;

		int solidTrianglesCount = 0;
		int liquidTrianglesCount = 0;
		int shadowTrianglesCount = 0;

    void setup(	std::vector<Texture*> textures, std::vector<Brush*>  brushes) {

        camera.quaternion =   glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1))
   	    					* glm::angleAxis(glm::radians(145.0f), glm::vec3(1, 0, 0))
   	    					* glm::angleAxis(glm::radians(135.0f), glm::vec3(0, 1, 0));  
		camera.position = glm::vec3(48,48,48);
        light.direction = glm::normalize(glm::vec3(-1.0,-1.0,-1.0));


		solidSpace = new Octree(BoundingCube(glm::vec3(0,0,0), 2.0));
		liquidSpace = new Octree(BoundingCube(glm::vec3(0,20,0), 2.0));
  
		BoundingBox mapBox(glm::vec3(-100,-60,-100), glm::vec3(100,50,100));
		HeightFunction * function = new GradientPerlinSurface(100, 1.0f/128.0f, 0);
		CachedHeightMapSurface * surface = new CachedHeightMapSurface(function, mapBox, solidSpace->minSize);
		HeightMap map(surface, mapBox.getMin(),mapBox.getMax(), solidSpace->minSize);

		solidSpace->add(new HeightMapContainmentHandler(&map, new LandBrush(brushes)));
		//tree->del(new SphereContainmentHandler(BoundingSphere(glm::vec3(00,-30,0),50), textures[7]));
		solidSpace->add(new SphereContainmentHandler(BoundingSphere(glm::vec3(0,50,0),20), new SimpleBrush(textures[6])));
		solidSpace->add(new SphereContainmentHandler(BoundingSphere(glm::vec3(-11,61,11),10), new SimpleBrush(textures[5])));
		solidSpace->del(new SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),10), new SimpleBrush(textures[4])));
		solidSpace->add(new BoxContainmentHandler(BoundingBox(glm::vec3(-10,6,-10),glm::vec3(34,50,34)),new SimpleBrush(textures[8])));
		solidSpace->del(new SphereContainmentHandler(BoundingSphere(glm::vec3(4,54,-4),8), new SimpleBrush(textures[1])));
		solidSpace->add(new SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),4), new SimpleBrush(textures[16])));

		BoundingBox waterBox(glm::vec3(-100,-60,-100), glm::vec3(100,3,100));
		//liquidSpace->add(new OctreeContainmentHandler(solidSpace, waterBox, new SimpleBrush(textures[6])));
		//BoundingBox waterBox(glm::vec3(50,50,0), glm::vec3(70,70,20));
		liquidSpace->add(new BoxContainmentHandler(waterBox, new WaterBrush(textures[16])));


		solidRenderer = new OctreeRenderer(solidSpace, &solidTrianglesCount, TYPE_SOLID_DRAWABLE, 5, 0.9, 0.2, true);
		liquidRenderer = new OctreeRenderer(liquidSpace, &liquidTrianglesCount, TYPE_LIQUID_DRAWABLE, 5, 0.9, 0.2, true);
		shadowRenderer = new OctreeRenderer(solidSpace, &shadowTrianglesCount, TYPE_SHADOW_DRAWABLE, 6, 0.1, 4.0, false);
    }

	void draw3dShadow() {
		solidSpace->iterate(shadowRenderer);
	}

	void update3d(glm::mat4 mvp, glm::mat4 mlp) {
		solidRenderer->loaded = 0;
		liquidRenderer->loaded = 0;
		shadowRenderer->loaded = 0;

		solidRenderer->cameraPosition = camera.position;
		liquidRenderer->cameraPosition = camera.position;
		shadowRenderer->cameraPosition = camera.position -light.direction*512.0f;
	
		solidRenderer->update(mvp);
		solidRenderer->mode = GL_PATCHES;

		liquidRenderer->update(mvp);
		liquidRenderer->mode = GL_PATCHES;

		shadowRenderer->update(mlp);
		shadowRenderer->mode = GL_TRIANGLES;
	}


	void draw3dSolid() {
		solidSpace->iterate(solidRenderer);
	}

	void draw3dLiquid() {
		liquidSpace->iterate(liquidRenderer);
	}


	void save() {
		OctreeSaver saver(solidSpace, "solid.bin");
		solidSpace->iterate(&saver);
		saver.close();
	}
};
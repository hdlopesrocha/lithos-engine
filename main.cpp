
#include "gl/gl.hpp"
#include <math.h>
#include "math/math.hpp"


class MainApplication : public LithosApplication {
	std::vector<Image> images;
  	Camera camera;
	Octree * tree;

public:
	MainApplication() {

	}

	~MainApplication() {

	}

    virtual void setup() {
        images.push_back(loadTextureImage("textures/pixel.jpg"));
        images.push_back(loadTextureImage("textures/grid.png"));
        images.push_back(loadTextureImage("textures/grass.png"));
        images.push_back(loadTextureImage("textures/sand.png"));
        images.push_back(loadTextureImage("textures/rock.png"));
        images.push_back(loadTextureImage("textures/snow.png"));
        images.push_back(loadTextureImage("textures/lava.png"));
        images.push_back(loadTextureImage("textures/dirt.png"));
        images.push_back(loadTextureImage("textures/grid3.png"));

        camera.quaternion =   glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1))
   	    					* glm::angleAxis(glm::radians(145.0f), glm::vec3(1, 0, 0))
   	    					* glm::angleAxis(glm::radians(135.0f), glm::vec3(0, 1, 0));  
		camera.pos = glm::vec3(48,48,48);

		tree = new Octree(2.0);

		BoundingSphere sph(glm::vec3(0,0,0),20);
		tree->add(new SphereContainmentHandler(sph, 8));

		Tesselator tesselator(tree);
		tree->iterate(&tesselator);
    }

    virtual void draw() {

    }

    virtual void update(float deltaTime){
	   	if (getKeyboardStatus(GLFW_KEY_ESCAPE) != GLFW_RELEASE) {
		   	close();
		}

        camera.projection = glm::perspective(glm::radians(45.0f), getWidth() / (float) getHeight(), 0.1f, 512.0f);
	    camera.projection[1][1] *= -1;
    
	 //   modelMatrix = glm::rotate(modelMatrix, deltaTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	   	float rsense = 0.005;

	   	if (getKeyboardStatus(GLFW_KEY_W) != GLFW_RELEASE) {
	   	   	camera.quaternion = glm::angleAxis(+rsense, glm::vec3(1,0,0))*camera.quaternion;
		}
	   	if (getKeyboardStatus(GLFW_KEY_S) != GLFW_RELEASE) {
	   	   	camera.quaternion = glm::angleAxis(-rsense, glm::vec3(1,0,0))*camera.quaternion;
		}
   		if (getKeyboardStatus(GLFW_KEY_A) != GLFW_RELEASE) {
	   	   	camera.quaternion = glm::angleAxis(-rsense, glm::vec3(0,1,0))*camera.quaternion;
		}
	   	if (getKeyboardStatus(GLFW_KEY_D) != GLFW_RELEASE) {
	   	   	camera.quaternion = glm::angleAxis(+rsense, glm::vec3(0,1,0))*camera.quaternion;
		}
		if (getKeyboardStatus(GLFW_KEY_Q) != GLFW_RELEASE) {
	   	   	camera.quaternion = glm::angleAxis(+rsense, glm::vec3(0,0,1))*camera.quaternion;
		}
	   	if (getKeyboardStatus(GLFW_KEY_E) != GLFW_RELEASE) {
	   	   	camera.quaternion = glm::angleAxis(-rsense, glm::vec3(0,0,1))*camera.quaternion;
		}

		glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f)*camera.quaternion;
		glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f)*camera.quaternion;
		glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f)*camera.quaternion;

	   	float tsense = deltaTime*20;
	   	if (getKeyboardStatus(GLFW_KEY_UP) != GLFW_RELEASE) {
	   		camera.pos -= zAxis*tsense;
		}
	   	if (getKeyboardStatus(GLFW_KEY_DOWN) != GLFW_RELEASE) {
	   		camera.pos += zAxis*tsense;
		}
	   	if (getKeyboardStatus(GLFW_KEY_RIGHT) != GLFW_RELEASE) {
	   		camera.pos += xAxis*tsense;
		}
   		if (getKeyboardStatus(GLFW_KEY_LEFT) != GLFW_RELEASE) {
	   		camera.pos -= xAxis*tsense;
		}

		camera.quaternion = glm::normalize(camera.quaternion);
		glm::mat4 rotate = glm::mat4_cast(camera.quaternion);
		glm::mat4 translate = glm::translate(glm::mat4(1.0f), -camera.pos);
	    camera.view = rotate * translate;

    }

    virtual void clean(){

    }

};

int main() {
    MainApplication app;
    app.run();
    return 0;
}

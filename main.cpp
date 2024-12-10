
#include "gl/gl.hpp"
#include <math.h>
#include <glm/gtc/type_ptr.hpp>

#include "math/math.hpp"


class MainApplication : public LithosApplication {
	std::vector<Image> images;
  	Camera camera;
	Octree * tree;
	Tesselator * tesselator;
	Tesselator * tesselator2;

	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint shaderProgram;
	GLuint modelLoc;
	GLuint viewLoc;
	GLuint projectionLoc;
	GLuint vao, vbo, ebo;

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

		std::string vertCode = readFile("shaders/vertShader.glsl");
		std::string fragCode = readFile("shaders/fragShader.glsl");
		vertexShader = compileShader(vertCode,GL_VERTEX_SHADER);
		fragmentShader = compileShader(fragCode,GL_FRAGMENT_SHADER);
		shaderProgram = createShaderProgram(vertexShader, fragmentShader);
		modelLoc = glGetUniformLocation(shaderProgram, "model");
		viewLoc = glGetUniformLocation(shaderProgram, "view");
		projectionLoc = glGetUniformLocation(shaderProgram, "projection");


		// Use the shader program
		glUseProgram(shaderProgram);


        camera.quaternion =   glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1))
   	    					* glm::angleAxis(glm::radians(145.0f), glm::vec3(1, 0, 0))
   	    					* glm::angleAxis(glm::radians(135.0f), glm::vec3(0, 1, 0));  
		camera.pos = glm::vec3(48,48,48);

		tree = new Octree(2.0);

		BoundingSphere sph(glm::vec3(0,0,0),20);
		tree->add(new SphereContainmentHandler(sph, 8));

		tesselator = new Tesselator(tree);
		tesselator2 = new Tesselator(tree);
		tree->iterate(tesselator);




		tesselator2->vertices.push_back(Vertex(glm::vec3(0.0f, 0.0f, 0.0f),glm::vec3(1.0f, 0.0f, 0.0f),glm::vec2(), 0));
		tesselator2->vertices.push_back(Vertex(glm::vec3(1.0f, 0.0f, 0.0f),glm::vec3(0.0f, 1.0f, 0.0f),glm::vec2(), 0));
		tesselator2->vertices.push_back(Vertex(glm::vec3(1.0f, 1.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(), 0));
		tesselator2->vertices.push_back(Vertex(glm::vec3(0.0f, 1.0f, 0.0f),glm::vec3(1.0f, 1.0f, 0.0f),glm::vec2(), 0));

		tesselator2->vertices.push_back(Vertex(glm::vec3(0.0f, 0.0f, 1.0f),glm::vec3(1.0f, 0.0f, 1.0f),glm::vec2(), 0));
		tesselator2->vertices.push_back(Vertex(glm::vec3(1.0f, 0.0f, 1.0f),glm::vec3(0.0f, 1.0f, 1.0f),glm::vec2(), 0));
		tesselator2->vertices.push_back(Vertex(glm::vec3(1.0f, 1.0f, 1.0f),glm::vec3(1.0f, 1.0f, 1.0f),glm::vec2(), 0));
		tesselator2->vertices.push_back(Vertex(glm::vec3(0.0f, 1.0f, 1.0f),glm::vec3(0.0f, 0.0f, 0.0f),glm::vec2(), 0));


    // Front face
    tesselator2->indices.push_back(0);
	tesselator2->indices.push_back(1);
	tesselator2->indices.push_back(2);
	tesselator2->indices.push_back(0);
	tesselator2->indices.push_back(2);
	tesselator2->indices.push_back(3);
    // Back face
     
	tesselator2->indices.push_back(4); 
	tesselator2->indices.push_back(6); 
	tesselator2->indices.push_back(5); 
	tesselator2->indices.push_back(4); 
	tesselator2->indices.push_back(7); 
	tesselator2->indices.push_back(6);
    // Left face
     
	tesselator2->indices.push_back(4); 
	tesselator2->indices.push_back(0); 
	tesselator2->indices.push_back(3); 
	tesselator2->indices.push_back(4); 
	tesselator2->indices.push_back(3); 
	tesselator2->indices.push_back(7);
    // Right face
     
	tesselator2->indices.push_back(1); 
	tesselator2->indices.push_back(5); 
	tesselator2->indices.push_back(6); 
	tesselator2->indices.push_back(1); 
	tesselator2->indices.push_back(6); 
	tesselator2->indices.push_back(2);
    // Top face
     
	tesselator2->indices.push_back(3); 
	tesselator2->indices.push_back(2); 
	tesselator2->indices.push_back(6); 
	tesselator2->indices.push_back(3); 
	tesselator2->indices.push_back(6); 
	tesselator2->indices.push_back(7);
    // Bottom face
     
	tesselator2->indices.push_back(4); 
	tesselator2->indices.push_back(5); 
	tesselator2->indices.push_back(1); 
	tesselator2->indices.push_back(4);  
	tesselator2->indices.push_back(1);  
	tesselator2->indices.push_back(0);

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);
	
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, tesselator2->vertices.size()*sizeof(Vertex), tesselator2->vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, tesselator2->indices.size()*sizeof(uint16_t), tesselator2->indices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (sizeof(float)*3));
		glEnableVertexAttribArray(1);

		glClearColor (0.1,0.1,0.1,1.0);
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK); // Or GL_FRONT
		glFrontFace(GL_CCW); // Ensure this matches your vertex data

		glEnable(GL_DEBUG_OUTPUT);
glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    printf("GL DEBUG: %s\n", message);
}, nullptr);
    }

    virtual void draw() {
		//std::cout << "Drawing #indices = "<< tesselator2->indices.size() << std::endl;
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, tesselator2->indices.size(), GL_UNSIGNED_INT, 0);
		GLenum error = glGetError();
if (error != GL_NO_ERROR) {
    printf("OpenGL Error: %d\n", error);
}
    }

    virtual void update(float deltaTime){
	   	if (getKeyboardStatus(GLFW_KEY_ESCAPE) != GLFW_RELEASE) {
		   	close();
		}

        camera.projection = glm::perspective(glm::radians(45.0f), getWidth() / (float) getHeight(), 0.1f, 512.0f);
	//    camera.projection[1][1] *= -1;
    
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


    glm::vec3 eye = glm::vec3(-2.0f, 2.0f, 2.0f);
    glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 viewMatrix = glm::lookAt(eye, center, up);



		glm::mat4 model = glm::mat4(1.0f); // Identity matrix

		// Send matrices to the shader
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(camera.projection));
    }

    virtual void clean(){

		// Cleanup and exit
		glDeleteProgram(shaderProgram);
    }

};

int main() {
    MainApplication app;
    app.run();
    return 0;
}


#include "gl/gl.hpp"
#include <math.h>
#include <glm/gtc/type_ptr.hpp>

#include "math/math.hpp"


class MainApplication : public LithosApplication {
	std::vector<Image> images;
  	Camera camera;
	Octree * tree;
	Tesselator * tesselator;

	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint shaderProgram;
	GLuint modelLoc;
	GLuint viewLoc;
	GLuint projectionLoc;
	GLuint lightDirectionLoc;
	GLuint vao, vbo, ebo;

public:
	MainApplication() {

	}

	~MainApplication() {

	}

    virtual void setup() {
glEnable(GL_DEPTH_TEST);
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

		// Use the shader program
		glUseProgram(shaderProgram);

		modelLoc = glGetUniformLocation(shaderProgram, "model");
		viewLoc = glGetUniformLocation(shaderProgram, "view");
		projectionLoc = glGetUniformLocation(shaderProgram, "projection");
		lightDirectionLoc = glGetUniformLocation(shaderProgram, "lightDirection");

		for(int i=0 ; i < images.size() ; ++i) {
			glActiveTexture(GL_TEXTURE0 + i); 
		    glBindTexture(GL_TEXTURE_2D, images[i].textureID);    // Bind the texture to the active unit
			GLint texLocation = glGetUniformLocation(shaderProgram, ("textures[" + std::to_string(i) + "]").c_str());
		    glUniform1i(texLocation, i);
		}

		



        camera.quaternion =   glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1))
   	    					* glm::angleAxis(glm::radians(145.0f), glm::vec3(1, 0, 0))
   	    					* glm::angleAxis(glm::radians(135.0f), glm::vec3(0, 1, 0));  
		camera.pos = glm::vec3(48,48,48);

		tree = new Octree(2.0);

		BoundingSphere sph(glm::vec3(0,0,0),20);
		tree->add(new SphereContainmentHandler(sph, 8));

		tesselator = new Tesselator(tree);
		tree->iterate(tesselator);

		for(int i=0; i < tesselator->vertices.size(); ++i) {
			Vertex v = tesselator->vertices[i];
			//std::cout << v.toString() << std::endl;
		}

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, tesselator->vertices.size()*sizeof(Vertex), tesselator->vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, tesselator->indices.size()*sizeof(uint16_t), tesselator->indices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, pos));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoord));
		glEnableVertexAttribArray(2);		
		glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*) offsetof(Vertex, texIndex) );
		glEnableVertexAttribArray(3);

		glClearColor (0.1,0.1,0.1,1.0);
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK); // Or GL_FRONT
		glFrontFace(GL_CCW); // Ensure this matches your vertex data
    }

    virtual void draw() {
		glViewport(0, 0, getWidth(), getHeight());

		
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, tesselator->indices.size(), GL_UNSIGNED_SHORT, 0);
		


		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

		glm::mat4 model = glm::mat4(1.0f); // Identity matrix

		// Send matrices to the shader
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camera.view));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(camera.projection));
		glUniform3f(lightDirectionLoc, -1.0,-1.0,-1.0);
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

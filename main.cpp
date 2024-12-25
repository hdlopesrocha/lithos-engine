
#include "gl/gl.hpp"
#include <math.h>
#include <glm/gtc/type_ptr.hpp>
#include "DebugTesselator.hpp"
#include "math/math.hpp"

//#define DEBUG_GEO 0

class Geometry {
	public:
	GLuint vao, vbo, ebo;
};

class HeightMapContainmentHandler : public ContainmentHandler {

	public:
		HeightMap * map;
		unsigned char texture;
		unsigned char textureOut;

		HeightMapContainmentHandler(HeightMap * m, unsigned char t, unsigned char o) : ContainmentHandler(){
			this->map = m;
			this->texture = t;
			this->textureOut = o;
		}

		glm::vec3 getCenter() {
			return map->getCenter();
		}
		glm::vec3 getPoint(float x, float z) {
			return glm::vec3(x, map->getHeightAt(x,z) ,z);
		}

		ContainmentType check(BoundingCube cube, Vertex * vertex) {
			ContainmentType result = map->contains(cube); 
				
			if(result == ContainmentType::Intersects) {
				glm::vec3 c = cube.getCenter();
				glm::vec3 a = map->getCenter();
				glm::vec3 p0 = getPoint(c[0], c[2]); 
		
				if(map->hitsBoundary(cube) && cube.getMaxY() <= p0[1]) {
					vertex->pos = cube.getCenter();
					vertex->texIndex = this->textureOut;
					glm::vec3 n = glm::normalize(c-a);
					vertex->normal = n;
				} else {
					glm::vec3 p1 = getPoint(c[0]+1, c[2]); 
					glm::vec3 p2 = getPoint(c[0], c[2]+1); 
					glm::vec3 v1 = p1 - p0;
					glm::vec3 v2 = p2 - p0;

					vertex->normal = glm::cross(v2,v1);
					vertex->pos = p0;
					vertex->texIndex = this->texture;
				}
			}
			return result;
		}
};


class MainApplication : public LithosApplication {
	std::vector<Image> images;
  	Camera camera;
	Octree * tree;
	Tesselator * tesselator;
	#ifdef DEBUG_GEO
	DebugTesselator * debugTesselator;
	Geometry vaoDebug;
	#endif

	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint tessControlShader;
	GLuint tessEvaluationShader;

	GLuint shaderProgram;
	GLuint modelLoc;
	GLuint viewLoc;
	GLuint projectionLoc;
	GLuint lightDirectionLoc;
	GLuint lightEnabledLoc;
	GLuint triplanarEnabledLoc;
	Geometry vertexArrayObject;


public:
	MainApplication() {

	}

	~MainApplication() {

	}

	Geometry tesselatorToGeometry(TesselatorHandler * t){
		Geometry geo;
		glGenVertexArrays(1, &geo.vao);
		glGenBuffers(1, &geo.vbo);
		glGenBuffers(1, &geo.ebo);

		glBindVertexArray(geo.vao);
		glBindBuffer(GL_ARRAY_BUFFER, geo.vbo);
		glBufferData(GL_ARRAY_BUFFER, t->vertices.size()*sizeof(Vertex), t->vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, t->indices.size()*sizeof(uint), t->indices.data(), GL_STATIC_DRAW);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, pos));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoord));
		glEnableVertexAttribArray(2);		
		glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*) offsetof(Vertex, texIndex) );
		glEnableVertexAttribArray(3);
		
		return geo;
	}

    virtual void setup() {
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE); // Enable face culling
		glCullFace(GL_BACK); // Or GL_FRONT
		glFrontFace(GL_CCW); // Ensure this matches your vertex data
   


        images.push_back(loadTextureImage("textures/pixel.jpg"));
        images.push_back(loadTextureImage("textures/grid.png"));
        images.push_back(loadTextureImage("textures/grass.png"));
        images.push_back(loadTextureImage("textures/sand.png"));
        images.push_back(loadTextureImage("textures/rock.png"));
        images.push_back(loadTextureImage("textures/snow.png"));
        images.push_back(loadTextureImage("textures/lava.png"));
        images.push_back(loadTextureImage("textures/dirt.png"));
        images.push_back(loadTextureImage("textures/grid3.png"));
        images.push_back(loadTextureImage("textures/gridRed.png"));

		std::string vertCode = readFile("shaders/vertex.glsl");
		std::string fragCode = readFile("shaders/fragment.glsl");
		std::string controlCode = readFile("shaders/tessControl.glsl");
		std::string evalCode = readFile("shaders/tessEvaluation.glsl");



		vertexShader = compileShader(vertCode,GL_VERTEX_SHADER);
		fragmentShader = compileShader(fragCode,GL_FRAGMENT_SHADER);
		tessControlShader = compileShader(controlCode,GL_TESS_CONTROL_SHADER);
		tessEvaluationShader = compileShader(evalCode,GL_TESS_EVALUATION_SHADER);




		shaderProgram = createShaderProgram(vertexShader, fragmentShader, tessControlShader, tessEvaluationShader);

		// Use the shader program
		glUseProgram(shaderProgram);

		modelLoc = glGetUniformLocation(shaderProgram, "model");
		viewLoc = glGetUniformLocation(shaderProgram, "view");
		projectionLoc = glGetUniformLocation(shaderProgram, "projection");
		lightDirectionLoc = glGetUniformLocation(shaderProgram, "lightDirection");
		lightEnabledLoc = glGetUniformLocation(shaderProgram, "lightEnabled");
		triplanarEnabledLoc = glGetUniformLocation(shaderProgram, "triplanarEnabled");



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

		tree = new Octree(1.0);

		HeightMap map(glm::vec3(-64,-32,-64),glm::vec3(64,-16,64), 128, 128);
		tree->add(new HeightMapContainmentHandler(&map, 2, 7));

		BoundingSphere sph(glm::vec3(0,0,0),20);
		tree->add(new SphereContainmentHandler(sph, 2));

		BoundingSphere sph2(glm::vec3(-11,11,11),10);
		tree->add(new SphereContainmentHandler(sph2, 5));

		BoundingSphere sph3(glm::vec3(11,11,-11),10);
		tree->del(new SphereContainmentHandler(sph3, 4));

		BoundingSphere sph4(glm::vec3(4,4,-4),8);
		//tree->del(new SphereContainmentHandler(sph4, 6));

		BoundingSphere sph5(glm::vec3(11,11,-11),4);
		//tree->add(new SphereContainmentHandler(sph5, 3));

		BoundingBox box1(glm::vec3(0,-24,0),glm::vec3(24,0,24));
		tree->add(new BoxContainmentHandler(box1,4));


		tesselator = new Tesselator(tree);
		tree->iterate((IteratorHandler*)tesselator);

		#ifdef DEBUG_GEO
		debugTesselator = new DebugTesselator(tree);
		tree->iterate((IteratorHandler*)debugTesselator);
		#endif


		vertexArrayObject = tesselatorToGeometry(tesselator);
		#ifdef DEBUG_GEO
		vaoDebug = tesselatorToGeometry(debugTesselator);
		#endif

		glClearColor (0.1,0.1,0.1,1.0);



	 }

    virtual void draw() {
		glViewport(0, 0, getWidth(), getHeight());
		glEnable(GL_CULL_FACE);

		glUniform1ui(lightEnabledLoc, 1);
		glUniform1ui(triplanarEnabledLoc, 1);
		glPatchParameteri(GL_PATCH_VERTICES, 3); // Define the number of control points per patch

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBindVertexArray(vertexArrayObject.vao);
		glDrawElements(GL_PATCHES, tesselator->indices.size(), GL_UNSIGNED_INT, 0);



		glDisable(GL_CULL_FACE);
		glUniform1ui(lightEnabledLoc, 0);
    	glUniform1ui(triplanarEnabledLoc, 0);
		
		#ifdef DEBUG_GEO
		glBindVertexArray(vaoDebug.vao);
		glDrawElements(GL_TRIANGLES, debugTesselator->indices.size(), GL_UNSIGNED_INT, 0);
		#endif


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
		glUniform3fv(lightDirectionLoc, 1, glm::value_ptr(glm::normalize(glm::vec3(-1.0,-1.0,-1.0))));
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

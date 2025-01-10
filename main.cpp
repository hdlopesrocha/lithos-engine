
#include "gl/gl.hpp"
#include <math.h>
#include <glm/gtc/type_ptr.hpp>
#include "DebugTesselator.hpp"
#include "math/math.hpp"

//#define DEBUG_GEO 0
#define TEXTURES_COUNT 10


class Geometry {
	public:
	GLuint vao, vbo, ebo;
};

class Texture {
	public:
	Image texture;
	Image normal;
	Image bump;
	float parallax;
	
	Texture(Image texture) {
		this->texture = texture;
		this->normal = 0;
		this->bump = 0;
		this->parallax = 0;
	}

	Texture(Image texture, Image normal, Image bump, float parallax) {
		this->texture = texture;
		this->normal = normal;
		this->bump = bump;
		this->parallax = parallax;
	}
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
		
				if(map->hitsBoundary(cube)) {
					vertex->position = cube.getCenter();
					vertex->texIndex = this->textureOut;
				} else {
					glm::vec3 c = cube.getCenter();
					glm::vec3 p0 = getPoint(c.x, c.z); 
					vertex->position = p0;
					vertex->texIndex = this->texture;
				}
			}
			return result;
		}
};


class MainApplication : public LithosApplication {
	std::vector<Texture> textures;
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
	GLuint cameraPositionLoc;
	GLuint timeLoc;
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
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoord));
		glEnableVertexAttribArray(2);		
		glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*) offsetof(Vertex, texIndex) );
		glEnableVertexAttribArray(3);
	    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, tangent));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, bitangent));
		glEnableVertexAttribArray(5);	
		return geo;
	}


std::string replace(std::string input,  std::string replace_word, std::string replace_by ) {

 

    // Find the first occurrence of the substring
    size_t pos = input.find(replace_word);

    // Iterate through the string and replace all
    // occurrences
    while (pos != std::string::npos) {
        // Replace the substring with the specified string
        input.replace(pos, replace_word.size(), replace_by);

        // Find the next occurrence of the substring
        pos = input.find(replace_word,
                         pos + replace_by.size());
    }
	return input;
}

    virtual void setup() {
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE); // Enable face culling
		glCullFace(GL_BACK); // Or GL_FRONT
		glFrontFace(GL_CCW); // Ensure this matches your vertex data

        textures.push_back(Texture(loadTextureImage("textures/pixel.jpg")));
        textures.push_back(Texture(loadTextureImage("textures/grid.png")));
        textures.push_back(Texture(loadTextureImage("textures/grass_color.png"),loadTextureImage("textures/grass_normal.png"),loadTextureImage("textures/grass_bump.png"), 0.05 ));
        textures.push_back(Texture(loadTextureImage("textures/sand.png")));
        textures.push_back(Texture(loadTextureImage("textures/rock_color.png"),loadTextureImage("textures/rock_normal.png"),loadTextureImage("textures/rock_bump.png"), 0.1 ));
        textures.push_back(Texture(loadTextureImage("textures/snow_color.png"),loadTextureImage("textures/snow_normal.png"),loadTextureImage("textures/snow_bump.png"), 0.1 ));
        textures.push_back(Texture(loadTextureImage("textures/metal_color.png"),loadTextureImage("textures/metal_normal.png"),loadTextureImage("textures/metal_bump.png"), 0.3 ));
        textures.push_back(Texture(loadTextureImage("textures/dirt_color.png"),loadTextureImage("textures/dirt_normal.png"),loadTextureImage("textures/dirt_bump.png"), 0.1 ));
        textures.push_back(Texture(loadTextureImage("textures/bricks_color.png"),loadTextureImage("textures/bricks_normal.png"),loadTextureImage("textures/bricks_bump.png"), 0.05 ));

		std::string functionsLine = "#include<functions.glsl>";
		std::string functionsCode = readFile("shaders/functions.glsl");
		std::string vertCode = replace(readFile("shaders/vertex.glsl"), functionsLine, functionsCode);
		std::string fragCode = replace(readFile("shaders/fragment.glsl"), functionsLine, functionsCode);
		std::string controlCode = replace(readFile("shaders/tessControl.glsl"), functionsLine, functionsCode);
		std::string evalCode = replace(readFile("shaders/tessEvaluation.glsl"), functionsLine, functionsCode);

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
		cameraPositionLoc = glGetUniformLocation(shaderProgram, "cameraPosition");
		timeLoc = glGetUniformLocation(shaderProgram, "time");



		for(int i=0 ; i < textures.size() ; ++i) {
			glActiveTexture(GL_TEXTURE0 + i); 
		    Texture t = textures[i];

			glBindTexture(GL_TEXTURE_2D, t.texture);    // Bind the texture to the active unit
			GLint texLocation = glGetUniformLocation(shaderProgram, ("textures[" + std::to_string(i) + "]").c_str());
		    glUniform1i(texLocation, i);

			glActiveTexture(GL_TEXTURE0 + TEXTURES_COUNT+ i); 
			glBindTexture(GL_TEXTURE_2D, t.normal);    // Bind the texture to the active unit
			GLint normalLocation = glGetUniformLocation(shaderProgram, ("normalMaps[" + std::to_string(i) + "]").c_str());
		    glUniform1i(normalLocation, TEXTURES_COUNT + i);
		
			glActiveTexture(GL_TEXTURE0 + TEXTURES_COUNT*2+ i); 
			glBindTexture(GL_TEXTURE_2D, t.bump);    // Bind the texture to the active unit
			GLint bumpLocation = glGetUniformLocation(shaderProgram, ("bumpMaps[" + std::to_string(i) + "]").c_str());
		    glUniform1i(bumpLocation, TEXTURES_COUNT*2+  i);

			GLint parallaxLocation = glGetUniformLocation(shaderProgram, ("parallaxScale[" + std::to_string(i) + "]").c_str());
		    glUniform1f(parallaxLocation, t.parallax);

		}

        camera.quaternion =   glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1))
   	    					* glm::angleAxis(glm::radians(145.0f), glm::vec3(1, 0, 0))
   	    					* glm::angleAxis(glm::radians(135.0f), glm::vec3(0, 1, 0));  
		camera.position = glm::vec3(48,48,48);

		tree = new Octree(1.0);

		HeightMap map(glm::vec3(-64,-32,-64),glm::vec3(64,-16,64), 128, 128);
		tree->add(new HeightMapContainmentHandler(&map, 2, 7));

		BoundingSphere sph(glm::vec3(0,0,0),20);
		tree->add(new SphereContainmentHandler(sph, 6));

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
		tesselator->normalize();

		#ifdef DEBUG_GEO
		debugTesselator = new DebugTesselator(tree);
		tree->iterate((IteratorHandler*)debugTesselator);
		#endif


		vertexArrayObject = tesselatorToGeometry(tesselator);
		#ifdef DEBUG_GEO
		vaoDebug = tesselatorToGeometry(debugTesselator);
		#endif

		glClearColor (0.1,0.1,0.1,1.0);


		glUniform3fv(lightDirectionLoc, 1, glm::value_ptr(glm::normalize(glm::vec3(-1.0,-1.0,-1.0))));

	 }

    virtual void draw() {
		glViewport(0, 0, getWidth(), getHeight());
		glEnable(GL_CULL_FACE);

		glUniform1ui(lightEnabledLoc, 1);
		glUniform1ui(triplanarEnabledLoc, 0);
		glPatchParameteri(GL_PATCH_VERTICES, 3); // Define the number of control points per patch

//		glPolygonMode(GL_FRONT, GL_LINE);
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
float time = 0.0f;
    virtual void update(float deltaTime){
		time += deltaTime;
	   	if (getKeyboardStatus(GLFW_KEY_ESCAPE) != GLFW_RELEASE) {
		   	close();
		}

        camera.projection = glm::perspective(glm::radians(45.0f), getWidth() / (float) getHeight(), 0.1f, 512.0f);
	//    camera.projection[1][1] *= -1;
    
	 //   modelMatrix = glm::rotate(modelMatrix, deltaTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	   	float rsense = 0.01;

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

	   	float tsense = deltaTime*10;
	   	if (getKeyboardStatus(GLFW_KEY_UP) != GLFW_RELEASE) {
	   		camera.position -= zAxis*tsense;
		}
	   	if (getKeyboardStatus(GLFW_KEY_DOWN) != GLFW_RELEASE) {
	   		camera.position += zAxis*tsense;
		}
	   	if (getKeyboardStatus(GLFW_KEY_RIGHT) != GLFW_RELEASE) {
	   		camera.position += xAxis*tsense;
		}
   		if (getKeyboardStatus(GLFW_KEY_LEFT) != GLFW_RELEASE) {
	   		camera.position -= xAxis*tsense;
		}

		camera.quaternion = glm::normalize(camera.quaternion);
		glm::mat4 rotate = glm::mat4_cast(camera.quaternion);
		glm::mat4 translate = glm::translate(glm::mat4(1.0f), -camera.position);
	    camera.view = rotate * translate;

		glm::mat4 model = glm::mat4(1.0f); // Identity matrix

		// Send matrices to the shader
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camera.view));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(camera.projection));
		//glUniform3fv(lightDirectionLoc, 1, glm::value_ptr(glm::normalize(glm::vec3(glm::sin(time),-1.0,glm::cos(time)))));
		glUniform3fv(cameraPositionLoc, 1, glm::value_ptr(camera.position));
		glUniform1f(timeLoc, time);
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

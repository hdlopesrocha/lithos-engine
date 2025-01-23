
#include "gl/gl.hpp"
#include <math.h>
#include <glm/gtc/type_ptr.hpp>
#include "DebugTesselator.hpp"
#include "math/math.hpp"

//#define DEBUG_GEO 1

class Texture {
	public:
	int index;
	Image texture;
	Image normal;
	Image bump;
	float parallaxScale;
	float parallaxMinLayers;
	float parallaxMaxLayers;
	float shininess;

	Texture(Image texture) {
		this->texture = texture;
		this->normal = 0;
		this->bump = 0;
		this->parallaxScale = 0;
		this->parallaxMinLayers = 0;
		this->parallaxMaxLayers = 0;
		this->shininess = 0;
	}

	Texture(Image texture, Image normal, Image bump, float parallaxScale, float parallaxMinLayers, float parallaxMaxLayers, float shininess) {
		this->texture = texture;
		this->normal = normal;
		this->bump = bump;
		this->parallaxScale = parallaxScale;
		this->parallaxMinLayers = parallaxMinLayers;
		this->parallaxMaxLayers = parallaxMaxLayers;
		this->shininess = shininess;
	}
};

class SphereContainmentHandler : public ContainmentHandler {
	public:
	BoundingSphere sphere;
	Texture * texture;

	SphereContainmentHandler(BoundingSphere s, Texture * t) : ContainmentHandler(){
		this->sphere = s;
		this->texture = t;
	}

	glm::vec3 getCenter() {
		return sphere.center;
	}

	bool contains(glm::vec3 p) {
		return sphere.contains(p);
	}

	bool isContained(BoundingCube p) {
		return p.contains(sphere);
	}

	glm::vec3 getNormal(glm::vec3 pos) {
		return glm::normalize( pos - sphere.center);
	}

	ContainmentType check(BoundingCube cube) {
		return sphere.test(cube); 
	}

	Vertex getVertex(BoundingCube cube, ContainmentType solid) {
		Vertex vertex(cube.getCenter());
		glm::vec3 c = this->sphere.center;
		float r = this->sphere.radius;
		glm::vec3 a = cube.getCenter();
		glm::vec3 n = glm::normalize(a-c);
		glm::vec3 p = c + n*r;
		vertex.position = glm::clamp(p, cube.getMin(), cube.getMax());
		vertex.normal = getNormal(vertex.position);
		vertex.texIndex = texture->index;
		vertex.parallaxScale = texture->parallaxScale;
		vertex.parallaxMinLayers = texture->parallaxMinLayers;
		vertex.parallaxMaxLayers = texture->parallaxMaxLayers;
		vertex.shininess = texture->shininess;
		return vertex;
	}

};

class BoxContainmentHandler : public ContainmentHandler {
	public: 
	BoundingBox box;
	Texture * texture;

	BoxContainmentHandler(BoundingBox b, Texture * t) : ContainmentHandler(){
		this->box = b;
		this->texture = t;
	}

	glm::vec3 getCenter() {
		return box.getCenter();
	}

	bool contains(glm::vec3 p) {
		return box.contains(p);
	}

	bool isContained(BoundingCube p) {
		return p.contains(box);
	}
	
	ContainmentType check(BoundingCube cube) {
		return box.test(cube); 
	}

	Vertex getVertex(BoundingCube cube, ContainmentType solid) {
		Vertex vertex(cube.getCenter());

		glm::vec3 min = this->box.getMin();
		glm::vec3 max = this->box.getMax();
		glm::vec3 c = cube.getCenter();
	
		vertex.position = glm::clamp(c, min, max);
		vertex.normal = Math::surfaceNormal(vertex.position, box);
		vertex.texIndex = texture->index;
		vertex.parallaxScale = texture->parallaxScale;
		vertex.parallaxMinLayers = texture->parallaxMinLayers;
		vertex.parallaxMaxLayers = texture->parallaxMaxLayers;	
		vertex.shininess = texture->shininess;	
		return vertex;
	}
};

class HeightMapContainmentHandler : public ContainmentHandler {
	public: 
	HeightMap * map;
	Texture * texture;
	Texture * textureOut;

	HeightMapContainmentHandler(HeightMap * m, Texture * t, Texture * o) : ContainmentHandler(){
		this->map = m;
		this->texture = t;
		this->textureOut = o;
	}

	glm::vec3 getCenter() {
		return map->getCenter();
	}

	bool contains(glm::vec3 p) {
		return map->contains(p);
	}

	bool isContained(BoundingCube p) {
		return map->isContained(p);
	}

	float intersection(glm::vec3 a, glm::vec3 b) {
		return 0;	
	} 

	glm::vec3 getNormal(glm::vec3 pos) {
		return map->getNormalAt(pos.x, pos.z);
	}

	ContainmentType check(BoundingCube cube) {
		return map->test(cube); 
	}

	Vertex getVertex(BoundingCube cube, ContainmentType solid) {
		Vertex vertex(cube.getCenter());

		Texture * t;
		if(map->hitsBoundary(cube)) {
			vertex.normal = Math::surfaceNormal(cube.getCenter(), *map);
			glm::vec3 c = cube.getCenter()+vertex.normal*cube.getLength();
			c = glm::clamp(c, map->getMin(), map->getMax() );
			c = glm::clamp(c,cube.getMin(), cube.getMax() );
			vertex.position = c;
			t = textureOut;
		} else {
			glm::vec3 c = glm::clamp(map->getPoint(cube), map->getMin(), map->getMax());
			vertex.position = c;
			vertex.normal = getNormal(vertex.position);
			t = texture;
		}

		vertex.texIndex = t->index;
		vertex.parallaxScale = t->parallaxScale;
		vertex.parallaxMinLayers = t->parallaxMinLayers;
		vertex.parallaxMaxLayers = t->parallaxMaxLayers;	
		vertex.shininess = t->shininess;
	
		return vertex;
	}
};


class MainApplication : public LithosApplication {
	std::vector<Texture*> textures;
  	Camera camera;
	Octree * tree;
	Tesselator * tesselator;
	OctreeRenderer * renderer;
	#ifdef DEBUG_GEO
	DrawableGeometry * vaoDebug;
	#endif


	GLuint program2D;
	GLuint program3D;
	GLuint programShadow;
	
	GLuint modelViewProjectionLoc;
	GLuint modelViewProjectionShadowLoc;


	
	GLuint lightDirectionLoc;
	GLuint lightEnabledLoc;
	GLuint debugEnabledLoc;
	GLuint triplanarEnabledLoc;
	GLuint parallaxEnabledLoc;
	GLuint cameraPositionLoc;
	GLuint timeLoc;
	GLuint screen2dVao;
	GLuint fullSreenVao;
	//GLuint depthTexture;
	float time = 0.0f;


public:
	MainApplication() {

	}

	~MainApplication() {

	}

	std::string replace(std::string input,  std::string replace_word, std::string replace_by ) {
		size_t pos = input.find(replace_word);
		while (pos != std::string::npos) {
			input.replace(pos, replace_word.size(), replace_by);
			pos = input.find(replace_word, pos + replace_by.size());
		}
		return input;
	}

	void bindTextures(std::vector<Texture*> ts) {
		int activeTexture = 0;
		for(int i=0 ; i < ts.size() ; ++i) {
		    Texture * t = ts[i];
			t->index = i;
			glActiveTexture(GL_TEXTURE0 + activeTexture); 
			glBindTexture(GL_TEXTURE_2D, t->texture);    // Bind the texture to the active unit
		    glUniform1i(glGetUniformLocation(program3D, ("textures[" + std::to_string(i) + "]").c_str()), activeTexture++);

			glActiveTexture(GL_TEXTURE0 + activeTexture); 
			glBindTexture(GL_TEXTURE_2D, t->normal);    // Bind the texture to the active unit
		    glUniform1i(glGetUniformLocation(program3D, ("normalMaps[" + std::to_string(i) + "]").c_str()), activeTexture++);
		
			glActiveTexture(GL_TEXTURE0 + activeTexture); 
			glBindTexture(GL_TEXTURE_2D, t->bump);    // Bind the texture to the active unit
		    glUniform1i(glGetUniformLocation(program3D, ("bumpMaps[" + std::to_string(i) + "]").c_str()), activeTexture++);

		}
	}

	GLuint create2DVAO(float w, float h) {
		float vertices[] = {
			// positions   // tex coords
			0.0f, 0.0f,    0.0f, 1.0f,
			0.0f, h,    0.0f, 0.0f,
			w, h,    1.0f, 0.0f,
			w, 0.0f,    1.0f, 1.0f

		};
		unsigned int indices[] = {
			0, 1, 2,
			2, 3, 0
		};

		unsigned int VBO, VAO, EBO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		// Vertex position
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// Texture coordinates
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);
		return VAO;
	}

    virtual void setup() {
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		/*depthTexture = createDepthTexture(getWidth(), getHeight());
		if(!configureFrameBuffer(frameBuffer, depthTexture)){
			return;
		}
*/
        textures.push_back(new Texture(loadTextureImage("textures/grid.png")));
        textures.push_back(new Texture(loadTextureImage("textures/lava_color.jpg"),loadTextureImage("textures/lava_normal.jpg"),loadTextureImage("textures/lava_bump.jpg"), 0.1, 8, 32 ,256));
        textures.push_back(new Texture(loadTextureImage("textures/grass_color.png"),loadTextureImage("textures/grass_normal.png"),loadTextureImage("textures/grass_bump.png"), 0.01, 8, 32 ,256));
        textures.push_back(new Texture(loadTextureImage("textures/sand_color.jpg"),loadTextureImage("textures/sand_normal.jpg"),loadTextureImage("textures/sand_bump.jpg"), 0.1, 8, 32 ,256));
        textures.push_back(new Texture(loadTextureImage("textures/rock_color.png"),loadTextureImage("textures/rock_normal.png"),loadTextureImage("textures/rock_bump.png"), 0.1, 8, 32,128));
        textures.push_back(new Texture(loadTextureImage("textures/snow_color.png"),loadTextureImage("textures/snow_normal.png"),loadTextureImage("textures/snow_bump.png"), 0.1, 8, 32, 32 ));
        textures.push_back(new Texture(loadTextureImage("textures/metal_color.png"),loadTextureImage("textures/metal_normal.png"),loadTextureImage("textures/metal_bump.png"), 0.1, 8, 64, 32 ));
        textures.push_back(new Texture(loadTextureImage("textures/dirt_color.png"),loadTextureImage("textures/dirt_normal.png"),loadTextureImage("textures/dirt_bump.png"), 0.1, 8, 32 , 256));
        textures.push_back(new Texture(loadTextureImage("textures/bricks_color.png"),loadTextureImage("textures/bricks_normal.png"),loadTextureImage("textures/bricks_bump.png"), 0.01, 8, 32, 256 ));

		std::string vertCodeShadow = readFile("shaders/shadow_vertex.glsl");
		std::string fragCodeShadow = readFile("shaders/shadow_fragment.glsl");
		GLuint vertexShaderShadow = compileShader(vertCodeShadow,GL_VERTEX_SHADER);
		GLuint fragmentShaderShadow = compileShader(fragCodeShadow,GL_FRAGMENT_SHADER);
		programShadow = createShaderProgram(vertexShaderShadow, fragmentShaderShadow, 0, 0);
		glUseProgram(programShadow);


		std::string vertCode2D = readFile("shaders/2d_vertex.glsl");
		std::string fragCode2D = readFile("shaders/2d_fragment.glsl");
		GLuint vertexShader2D = compileShader(vertCode2D,GL_VERTEX_SHADER);
		GLuint fragmentShader2D = compileShader(fragCode2D,GL_FRAGMENT_SHADER);
		program2D = createShaderProgram(vertexShader2D, fragmentShader2D, 0, 0);


		glUseProgram(program2D);
		screen2dVao = create2DVAO(100,100);
		fullSreenVao = create2DVAO(getWidth(), getHeight());

		std::string functionsLine = "#include<functions.glsl>";
		std::string functionsCode = readFile("shaders/functions.glsl");
		std::string vertCode = replace(readFile("shaders/3d_vertex.glsl"), functionsLine, functionsCode);
		std::string fragCode = replace(readFile("shaders/3d_fragment.glsl"), functionsLine, functionsCode);
		std::string controlCode = replace(readFile("shaders/3d_tessControl.glsl"), functionsLine, functionsCode);
		std::string evalCode = replace(readFile("shaders/3d_tessEvaluation.glsl"), functionsLine, functionsCode);

		GLuint vertexShader = compileShader(vertCode,GL_VERTEX_SHADER);
		GLuint fragmentShader = compileShader(fragCode,GL_FRAGMENT_SHADER);
		GLuint tessControlShader = compileShader(controlCode,GL_TESS_CONTROL_SHADER);
		GLuint tessEvaluationShader = compileShader(evalCode,GL_TESS_EVALUATION_SHADER);
		program3D = createShaderProgram(vertexShader, fragmentShader, tessControlShader, tessEvaluationShader);
		glUseProgram(program3D);

		// Use the shader program

		modelViewProjectionShadowLoc = glGetUniformLocation(programShadow, "modelViewProjection");
	
		modelViewProjectionLoc = glGetUniformLocation(program3D, "modelViewProjection");
		lightDirectionLoc = glGetUniformLocation(program3D, "lightDirection");
		lightEnabledLoc = glGetUniformLocation(program3D, "lightEnabled");
		debugEnabledLoc = glGetUniformLocation(program3D, "debugEnabled");
		triplanarEnabledLoc = glGetUniformLocation(program3D, "triplanarEnabled");
		parallaxEnabledLoc = glGetUniformLocation(program3D, "parallaxEnabled");
		cameraPositionLoc = glGetUniformLocation(program3D, "cameraPosition");
		timeLoc = glGetUniformLocation(program3D, "time");

		bindTextures(textures);

		


        camera.quaternion =   glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1))
   	    					* glm::angleAxis(glm::radians(145.0f), glm::vec3(1, 0, 0))
   	    					* glm::angleAxis(glm::radians(135.0f), glm::vec3(0, 1, 0));  
		camera.position = glm::vec3(48,48,48);

		tree = new Octree(2.0, 4);

		HeightMap map(glm::vec3(-64,-64,-64),glm::vec3(64,-16,64), 128, 128);
		tree->add(new HeightMapContainmentHandler(&map, textures[2], textures[7]));

		BoundingSphere sph(glm::vec3(0,0,0),20);
		tree->add(new SphereContainmentHandler(sph, textures[6]));

		BoundingSphere sph2(glm::vec3(-11,11,11),10);
		tree->add(new SphereContainmentHandler(sph2, textures[5]));

		BoundingSphere sph3(glm::vec3(11,11,-11),10);
		tree->del(new SphereContainmentHandler(sph3, textures[4]));

		BoundingBox box1(glm::vec3(0,-24,0),glm::vec3(24,0,24));
		tree->add(new BoxContainmentHandler(box1,textures[8]));

		BoundingSphere sph4(glm::vec3(4,4,-4),8);
		tree->del(new SphereContainmentHandler(sph4, textures[1]));

		BoundingSphere sph5(glm::vec3(11,11,-11),4);
		tree->add(new SphereContainmentHandler(sph5, textures[3]));

		tesselator = new Tesselator(tree);
		tree->iterate(tesselator);

		renderer = new OctreeRenderer(tree);
		//tesselator->normalize();

		#ifdef DEBUG_GEO
		DebugTesselator * debugTesselator = new DebugTesselator(tree);
		tree->iterate(debugTesselator);
		vaoDebug = new DrawableGeometry(&debugTesselator->chunk);
		#endif

		glClearColor (0.1,0.1,0.1,1.0);
		glUniform3fv(lightDirectionLoc, 1, glm::value_ptr(glm::normalize(glm::vec3(-1.0,-1.0,-1.0))));
	 }

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
	
	   	if (getKeyboardStatus(GLFW_KEY_R) == GLFW_RELEASE) {
			renderer->update(camera.projection *camera.view);
		}


    }

    virtual void draw() {
		glm::mat4 model = glm::mat4(1.0f); // Identity matrix
		renderer->loaded = 0;
		glm::mat4 rotate = glm::mat4_cast(camera.quaternion);
		glm::mat4 translate = glm::translate(glm::mat4(1.0f), -camera.position);
	    camera.view = rotate * translate;
		glm::mat4 mvp = camera.projection * camera.view * model;

		// ================
		// Shadow component
		// ================
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
		glViewport(0, 0, getWidth(), getHeight());
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(programShadow);
		glUniformMatrix4fv(modelViewProjectionShadowLoc, 1, GL_FALSE, glm::value_ptr(mvp));

		renderer->mode = GL_TRIANGLES;
		tree->iterate(renderer);
	

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
		glViewport(0, 0, getWidth(), getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ============
		// 3D component
		// ============
		glUseProgram(program3D);
	
		// Send matrices to the shader
		glUniformMatrix4fv(modelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(mvp));

	//	glUniform3fv(lightDirectionLoc, 1, glm::value_ptr(glm::normalize(glm::vec3(glm::sin(time),-1.0,glm::cos(time)))));
		glUniform3fv(cameraPositionLoc, 1, glm::value_ptr(camera.position));
		glUniform1f(timeLoc, time);
		glUniform1ui(lightEnabledLoc, 1);
		glUniform1ui(triplanarEnabledLoc, 1);
		glUniform1ui(parallaxEnabledLoc, 1);
		glUniform1ui(debugEnabledLoc, 0);
		glPatchParameteri(GL_PATCH_VERTICES, 3); // Define the number of control points per patch

		glEnable(GL_CULL_FACE); // Enable face culling
		glCullFace(GL_BACK); // Or GL_FRONT
		glFrontFace(GL_CCW); // Ensure this matches your vertex data
		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPolygonMode(GL_FRONT, GL_FILL);
		renderer->mode = GL_PATCHES;
		tree->iterate(renderer);

		#ifdef DEBUG_GEO
		glUniform1ui(debugEnabledLoc, 1);
		glUniform1ui(lightEnabledLoc, 0);
		glUniform1ui(parallaxEnabledLoc, 0);
    	glUniform1ui(triplanarEnabledLoc, 0);
		glDisable(GL_CULL_FACE); // Enable face culling
		glPolygonMode(GL_FRONT, GL_LINE);
		glLineWidth(2.0);
		glPointSize(4.0);
		tree->iterate(renderer);
		//vaoDebug->draw(false);
		//glDrawElements(GL_PATCHES, vaoDebug->indices, GL_UNSIGNED_INT, 0);
		#endif
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// ============
		// 2D component
		// ============
		glUseProgram(program2D);
		glm::mat4 projection = glm::ortho(0.0f, (float)getWidth(), (float)getHeight(), 0.0f, -1.0f, 1.0f);
		glUniformMatrix4fv(glGetUniformLocation(program2D, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glActiveTexture(GL_TEXTURE0); 
		glBindTexture(GL_TEXTURE_2D, frameTexture);
		glUniform1i(glGetUniformLocation(program2D, "texture1"), 0); // Set the sampler uniform

		glBindVertexArray(screen2dVao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// ==========
		// Final Pass
		// ==========
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originalFrameBuffer);
		glBindTexture(GL_TEXTURE_2D, frameTexture);
		glUniform1i(glGetUniformLocation(program2D, "texture1"), 0); // Set the sampler uniform
		glBindVertexArray(fullSreenVao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		

    }

    virtual void clean(){

		// Cleanup and exit
		glDeleteProgram(program3D);
    }

};

int main() {
    MainApplication app;
    app.run();
    return 0;
}

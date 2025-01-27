
#include "gl/gl.hpp"
#include <math.h>
#include <glm/gtc/type_ptr.hpp>
#include "DebugTesselator.hpp"
#include "math/math.hpp"

#include "HeightFunctions.hpp"

//#define DEBUG_GEO 1

class SimpleBrush : public TextureBrush {

	Texture * texture;

	public: 
	SimpleBrush(Texture * texture){
		this->texture = texture;
	}

	void paint(Vertex * vertex) {
		vertex->texIndex = texture->index;
		vertex->parallaxScale = texture->parallaxScale;
		vertex->parallaxMinLayers = texture->parallaxMinLayers;
		vertex->parallaxMaxLayers = texture->parallaxMaxLayers;	
		vertex->shininess = texture->shininess;
		vertex->specularStrength = texture->specularStrength;
	}
};

class LandBrush : public TextureBrush {

	Texture * underground;
	Texture * grass;
	Texture * sand;
	Texture * rock;
	Texture * snow;

	public: 
	LandBrush(Texture * underground, Texture * grass, Texture * sand, Texture * rock, Texture * snow){
		this->underground = underground;
		this->grass = grass;
		this->sand = sand;
		this->rock = rock;
		this->snow = snow;
	}


	void paint(Vertex * vertex) {
		Texture * texture;
		if (glm::dot(glm::vec3(0.0f,1.0f,0.0f), vertex->normal ) <=0 ){
			texture= underground;
		} else if(glm::dot(glm::vec3(0.0f,1.0f,0.0f), vertex->normal ) < 0.8 ){
			texture = rock;
		} else if(vertex->position.y < -45){
			texture = sand;
		} else if(vertex->position.y < -25){
			texture = grass;
		} else {
			texture = snow;
		}

		vertex->texIndex = texture->index;
		vertex->parallaxScale = texture->parallaxScale;
		vertex->parallaxMinLayers = texture->parallaxMinLayers;
		vertex->parallaxMaxLayers = texture->parallaxMaxLayers;	
		vertex->shininess = texture->shininess;
		vertex->specularStrength = texture->specularStrength;
	}
};

class MainApplication : public LithosApplication {
	std::vector<Texture*> textures;
  	Camera camera;
	DirectionalLight light;
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
	GLuint matrixShadowLoc;

	GLuint lightDirectionLoc;
	GLuint lightEnabledLoc;
	GLuint debugEnabledLoc;
	GLuint triplanarEnabledLoc;
	GLuint parallaxEnabledLoc;
	GLuint cameraPositionLoc;
	GLuint shadowMapLoc;
	GLuint noiseLoc;
	GLuint timeLoc;

	GLuint noiseTexture;
	GLuint screen2dVao;
	GLuint fullSreenVao;
	RenderBuffer depthFrameBuffer;
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
		glActiveTexture(GL_TEXTURE0 + activeTexture); 
		glBindTexture(GL_TEXTURE_2D, depthFrameBuffer.texture);
		glUniform1i(shadowMapLoc, activeTexture++);

		glActiveTexture(GL_TEXTURE0 + activeTexture); 
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		glUniform1i(noiseLoc, activeTexture++);
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

		depthFrameBuffer = createDepthFrameBuffer(2048, 2048);

        textures.push_back(new Texture(loadTextureImage("textures/grid.png")));
        textures.push_back(new Texture(loadTextureImage("textures/lava_color.jpg"),loadTextureImage("textures/lava_normal.jpg"),loadTextureImage("textures/lava_bump.jpg"), 0.1, 8, 32 ,256, 0.4));
        textures.push_back(new Texture(loadTextureImage("textures/grass_color.png"),loadTextureImage("textures/grass_normal.png"),loadTextureImage("textures/grass_bump.png"), 0.01, 8, 32 ,32, 0.03));
        textures.push_back(new Texture(loadTextureImage("textures/sand_color.jpg"),loadTextureImage("textures/sand_normal.jpg"),loadTextureImage("textures/sand_bump.jpg"), 0.05, 8, 32 ,32,0.02));
        textures.push_back(new Texture(loadTextureImage("textures/rock_color.png"),loadTextureImage("textures/rock_normal.png"),loadTextureImage("textures/rock_bump.png"), 0.1, 8, 32,128, 0.4));
        textures.push_back(new Texture(loadTextureImage("textures/snow_color.png"),loadTextureImage("textures/snow_normal.png"),loadTextureImage("textures/snow_bump.png"), 0.1, 8, 32, 32 , 0.4));
        textures.push_back(new Texture(loadTextureImage("textures/metal_color.png"),loadTextureImage("textures/metal_normal.png"),loadTextureImage("textures/metal_bump.png"), 0.1, 8, 64, 32, 0.6 ));
        textures.push_back(new Texture(loadTextureImage("textures/dirt_color.png"),loadTextureImage("textures/dirt_normal.png"),loadTextureImage("textures/dirt_bump.png"), 0.1, 8, 32 , 256, 0.02));
        textures.push_back(new Texture(loadTextureImage("textures/bricks_color.png"),loadTextureImage("textures/bricks_normal.png"),loadTextureImage("textures/bricks_bump.png"), 0.01, 8, 32, 256, 0.2 ));
		noiseTexture = loadTextureImage("textures/noise.png");

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
		screen2dVao = create2DVAO(200,200);
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
		matrixShadowLoc = glGetUniformLocation(program3D, "matrixShadow");
		lightDirectionLoc = glGetUniformLocation(program3D, "lightDirection");
		lightEnabledLoc = glGetUniformLocation(program3D, "lightEnabled");
		debugEnabledLoc = glGetUniformLocation(program3D, "debugEnabled");
		triplanarEnabledLoc = glGetUniformLocation(program3D, "triplanarEnabled");
		parallaxEnabledLoc = glGetUniformLocation(program3D, "parallaxEnabled");
		cameraPositionLoc = glGetUniformLocation(program3D, "cameraPosition");
		timeLoc = glGetUniformLocation(program3D, "time");
		shadowMapLoc = glGetUniformLocation(program3D, "shadowMap");
		noiseLoc = glGetUniformLocation(program3D, "noise");
		
		bindTextures(textures);
		
		


        camera.quaternion =   glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1))
   	    					* glm::angleAxis(glm::radians(145.0f), glm::vec3(1, 0, 0))
   	    					* glm::angleAxis(glm::radians(135.0f), glm::vec3(0, 1, 0));  
		camera.position = glm::vec3(48,48,48);

		tree = new Octree(3.0, 5);

		HeightMap map(new GradientPerlinSurface(100, 1.0f/128.0f, -50), glm::vec3(-200,-100,-200),glm::vec3(200,0,200), tree->minSize);

		tree->add(new HeightMapContainmentHandler(&map, new LandBrush(textures[7],textures[2],textures[3],textures[4],textures[5])));
		//tree->del(new SphereContainmentHandler(BoundingSphere(glm::vec3(00,-30,0),50), textures[7]));
		tree->add(new SphereContainmentHandler(BoundingSphere(glm::vec3(0,0,0),20), new SimpleBrush(textures[6])));
		tree->add(new SphereContainmentHandler(BoundingSphere(glm::vec3(-11,11,11),10), new SimpleBrush(textures[5])));
		tree->del(new SphereContainmentHandler(BoundingSphere(glm::vec3(11,11,-11),10), new SimpleBrush(textures[4])));
		tree->add(new BoxContainmentHandler(BoundingBox(glm::vec3(0,-24,0),glm::vec3(24,0,24)),new SimpleBrush(textures[8])));
		tree->del(new SphereContainmentHandler(BoundingSphere(glm::vec3(4,4,-4),8), new SimpleBrush(textures[1])));
		tree->add(new SphereContainmentHandler(BoundingSphere(glm::vec3(11,11,-11),4), new SimpleBrush(textures[3])));

		tesselator = new Tesselator(tree);
		tree->iterate(tesselator);

		renderer = new OctreeRenderer(tree);
		//tesselator->normalize();

		#ifdef DEBUG_GEO
		if(DEBUG_GEO == 0) {
			DebugTesselator * debugTesselator = new DebugTesselator(tree);
			tree->iterate(debugTesselator);
			vaoDebug = new DrawableGeometry(&debugTesselator->chunk);
		}
		#endif

		glClearColor (0.1,0.1,0.1,1.0);
        light.direction = glm::normalize(glm::vec3(-1.0,-1.0,-1.0));
	 
	 	std::cout << "Setup complete!" << std::endl;
		std::cout << "#triangles = " << tesselator->triangles << std::endl;
		
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
		glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f)*camera.quaternion;
	

		float far = 512.0f;
		float dist = 32.0f;
	   	glm::vec3 lookAtLightPosition = glm::round(camera.position/16.0f)*16.0f; // + cameraDirection*far*0.5f;


		light.direction = glm::normalize(glm::vec3(glm::sin(time),-1.0,glm::cos(time)));

		float orthoSize = 512.0f;  // Size of the orthographic box

		light.projection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, far);
		light.view = glm::lookAt(lookAtLightPosition - light.direction*dist, lookAtLightPosition, glm::vec3(0,1,0));
    }

    virtual void draw() {
		glm::mat4 model = glm::mat4(1.0f); // Identity matrix
		renderer->loaded = 0;
		glm::mat4 rotate = glm::mat4_cast(camera.quaternion);
		glm::mat4 translate = glm::translate(glm::mat4(1.0f), -camera.position);
	    camera.view = rotate * translate;
		glm::mat4 mvp = camera.projection * camera.view;
		glm::mat4 mlp = light.projection * light.view;

		// ================
		// Shadow component
		// ================
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, depthFrameBuffer.frameBuffer);
		glViewport(0, 0, depthFrameBuffer.width, depthFrameBuffer.height);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);
		glUseProgram(programShadow);
		glUniformMatrix4fv(modelViewProjectionShadowLoc, 1, GL_FALSE, glm::value_ptr(mlp));

		renderer->mode = GL_TRIANGLES;
		renderer->update(mlp);
		
		tree->iterate(renderer);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderBuffer.frameBuffer);
		glViewport(0, 0, getWidth(), getHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ============
		// 3D component
		// ============
		glUseProgram(program3D);
	    float bias = 0.003;
		glm::mat4 biasMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,0.0f,-bias));


		glm::mat4 ms =  glm::translate(glm::mat4(1.0f), glm::vec3(0.5)) 
						* glm::scale(glm::mat4(1.0f), glm::vec3(0.5)) 
						* mlp 					;

		// Send matrices to the shader
		glUniformMatrix4fv(modelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(matrixShadowLoc, 1, GL_FALSE, glm::value_ptr(ms  ));
		glUniform3fv(lightDirectionLoc, 1, glm::value_ptr(light.direction));
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
		renderer->update(mvp);
		tree->iterate(renderer);

		#ifdef DEBUG_GEO
	
		glUniform1ui(lightEnabledLoc, 0);
		glUniform1ui(parallaxEnabledLoc, 0);
    	glUniform1ui(triplanarEnabledLoc, 0);
	
		if(DEBUG_GEO == 1) {
			glUniform1ui(debugEnabledLoc, 1);
			glPolygonMode(GL_FRONT, GL_LINE);
			glLineWidth(2.0);
			glPointSize(4.0);	
			tree->iterate(renderer);
		} else {
			glDisable(GL_CULL_FACE); // Enable face culling
			vaoDebug->draw(GL_PATCHES);
		}
		
		
		glPolygonMode(GL_FRONT, GL_FILL);
		#endif

		// ============
		// 2D component
		// ============
		glUseProgram(program2D);
		glm::mat4 projection = glm::ortho(0.0f, (float)getWidth(), (float)getHeight(), 0.0f, -1.0f, 1.0f);
		glUniformMatrix4fv(glGetUniformLocation(program2D, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		// ==========
		// Final Pass
		// ==========
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originalFrameBuffer);
		glActiveTexture(GL_TEXTURE0); 
		glBindTexture(GL_TEXTURE_2D, renderBuffer.texture);
		glUniform1i(glGetUniformLocation(program2D, "texture1"), 0); // Set the sampler uniform
		
		glBindVertexArray(fullSreenVao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
		// ==========
		// 2D overlay
		// ==========
		glActiveTexture(GL_TEXTURE0); 
		glBindTexture(GL_TEXTURE_2D, depthFrameBuffer.texture);
		glUniform1i(glGetUniformLocation(program2D, "texture1"), 0); // Set the sampler uniform


		glUniform1f(glGetUniformLocation(program2D, "near"), 0.1f); 
		glUniform1f(glGetUniformLocation(program2D, "far"), 512.0f); 
		glUniform1i(glGetUniformLocation(program2D, "depthEnabled"), false);

		glBindVertexArray(screen2dVao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glUniform1i(glGetUniformLocation(program2D, "depthEnabled"), false); 

		glActiveTexture(GL_TEXTURE0); 
		glBindTexture(GL_TEXTURE_2D, textures[0]->texture);
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

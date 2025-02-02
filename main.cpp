#include "gl/gl.hpp"
#include <math.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/integer.hpp>
#include "DebugTesselator.hpp"
#include "math/math.hpp"
#include "ui/ui.hpp"
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
	}
};

class LandBrush : public TextureBrush {

	Texture * underground;
	Texture * grass;
	Texture * sand;
	Texture * rock;
	Texture * snow;
	Texture * grassMixSand;
	Texture * grassMixSnow;
	Texture * rockMixGrass;
	Texture * rockMixSnow;
	Texture * rockMixSand;
	public: 
	LandBrush(std::vector<Brush*> brushes){
		this->underground = brushes[7]->texture;
		this->grass = brushes[2]->texture;
		this->sand = brushes[3]->texture;
		this->rock = brushes[4]->texture;
		this->snow = brushes[5]->texture;
		this->grassMixSand = brushes[9]->texture;
		this->grassMixSnow = brushes[10]->texture;
	
		this->rockMixGrass = brushes[11]->texture;
		this->rockMixSnow = brushes[12]->texture;
		this->rockMixSand = brushes[13]->texture;
	}


	void paint(Vertex * vertex) {
		float steepness =glm::dot(glm::vec3(0.0f,1.0f,0.0f), vertex->normal );
		int grassLevel = 25;
		int sandLevel = 5;


		Texture * texture;
		if (glm::dot(glm::vec3(0.0f,1.0f,0.0f), vertex->normal ) <=0 ){
			texture= underground;
		} else if(steepness < 0.8 ){
			texture = rock;
		} else if(steepness < 0.9 ){
		    if(vertex->position.y < sandLevel){
				texture = rockMixSand;
			} else if(vertex->position.y < grassLevel){
				texture = rockMixGrass;
			} else {
				texture = rockMixSnow;
			}
		} else if(vertex->position.y < sandLevel){
			texture = sand;
		} else if(vertex->position.y < sandLevel+1){
			texture = grassMixSand;
		} else if(vertex->position.y < grassLevel){
			texture = grass;
		} else if(vertex->position.y < grassLevel+1){
			texture = grassMixSnow;
		} else {
			texture = snow;
		}

		vertex->texIndex = texture->index;
	}
};



class OctreeContainmentHandler : public ContainmentHandler {
	public:
	Octree * octree;
    TextureBrush * brush;
	BoundingBox box;

	OctreeContainmentHandler(Octree * octree, BoundingBox box, TextureBrush * b) {
		this->brush = b;
		this->octree = octree;
		this->box = box;
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

	glm::vec3 getNormal(glm::vec3 pos) {
		return Math::surfaceNormal(pos, box);
	}

	ContainmentType check(BoundingCube cube) {
		return box.test(cube);
	}


	Vertex getVertex(BoundingCube cube, ContainmentType solid) {
		Vertex vtx;
		vtx.position = cube.getCenter();
		return vtx;
	}
};

class GlslInclude {
	public:
	std::string line;
	std::string code;
	GlslInclude(std::string line, std::string code){
		this->line = line;
		this->code = code;
	}


};

std::string replace(std::string input,  std::string replace_word, std::string replace_by ) {
	size_t pos = input.find(replace_word);
	while (pos != std::string::npos) {
		input.replace(pos, replace_word.size(), replace_by);
		pos = input.find(replace_word, pos + replace_by.size());
	}
	return input;
}


std::string replaceIncludes(std::vector<GlslInclude> includes, std::string code){
	for(int i=0; i< includes.size() ;++i) {
		GlslInclude include = includes[i];
		code = replace(code, include.line, include.code);
	}
	return code;
}

class ProgramLocations {
	public:
	GLuint program;
	GLuint modelLoc;
	GLuint modelViewProjectionLoc;
	GLuint matrixShadowLoc;
	GLuint lightDirectionLoc;
	GLuint lightEnabledLoc;
	GLuint debugEnabledLoc;
	GLuint triplanarEnabledLoc;
	GLuint parallaxEnabledLoc;
	GLuint shadowEnabledLoc;
	GLuint depthTestEnabledLoc;
	GLuint depthTextureLoc;
	GLuint underTextureLoc;
	GLuint cameraPositionLoc;
	GLuint overrideTextureEnabledLoc;
	GLuint overrideTextureLoc;
	GLuint shadowMapLoc;
	GLuint noiseLoc;
	GLuint timeLoc;

	ProgramLocations(GLuint program) {
		this->program = program;
		this->modelLoc = glGetUniformLocation(program, "model");
		this->modelViewProjectionLoc = glGetUniformLocation(program, "modelViewProjection");
		this->matrixShadowLoc = glGetUniformLocation(program, "matrixShadow");
		this->lightDirectionLoc = glGetUniformLocation(program, "lightDirection");
		this->lightEnabledLoc = glGetUniformLocation(program, "lightEnabled");
		this->debugEnabledLoc = glGetUniformLocation(program, "debugEnabled");
		this->triplanarEnabledLoc = glGetUniformLocation(program, "triplanarEnabled");
		this->shadowEnabledLoc = glGetUniformLocation(program, "shadowEnabled");
		this->parallaxEnabledLoc = glGetUniformLocation(program, "parallaxEnabled");
		this->cameraPositionLoc = glGetUniformLocation(program, "cameraPosition");
		this->timeLoc = glGetUniformLocation(program, "time");
		this->shadowMapLoc = glGetUniformLocation(program, "shadowMap");
		this->noiseLoc = glGetUniformLocation(program, "noise");
		this->overrideTextureLoc = glGetUniformLocation(program, "overrideTexture");
		this->overrideTextureEnabledLoc = glGetUniformLocation(program, "overrideTextureEnabled");
		this->depthTestEnabledLoc = glGetUniformLocation(program, "depthTestEnabled");
		this->depthTextureLoc = glGetUniformLocation(program, "depthTexture");
		this->underTextureLoc = glGetUniformLocation(program, "underTexture");
	}

	void update(glm::mat4 modelViewProjection,glm::mat4 model, glm::mat4 matrixShadow, glm::vec3 lightDirection, glm::vec3 cameraPosition, float time) {
		glUniformMatrix4fv(modelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(modelViewProjection));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(matrixShadowLoc, 1, GL_FALSE, glm::value_ptr(matrixShadow  ));
		glUniform3fv(lightDirectionLoc, 1, glm::value_ptr(lightDirection));
		glUniform3fv(cameraPositionLoc, 1, glm::value_ptr(cameraPosition));
		glUniform1f(timeLoc, time);
		glUniform1ui(lightEnabledLoc, 1);
		glUniform1ui(triplanarEnabledLoc, 1);
		glUniform1ui(parallaxEnabledLoc, 1);
		glUniform1ui(debugEnabledLoc,0);
		glUniform1ui(shadowEnabledLoc, 1);
		glUniform1ui(depthTestEnabledLoc, 0);
	}

};

class MainApplication : public LithosApplication {
	std::vector<Texture*> textures;
	std::vector<Brush*> brushes;
	std::vector<TextureMixer*> mixers;
	std::vector<AnimatedTexture*> animatedTextures;

  	Camera camera;
	DirectionalLight light;
	Octree * solidSpace;
	Octree * liquidSpace;
	Tesselator * solidTesselator;
	Tesselator * liquidTesselator;
	OctreeRenderer * solidRenderer;
	OctreeRenderer * liquidRenderer;
	#ifdef DEBUG_GEO
	DrawableGeometry * vaoDebug;
	#endif


	GLuint programSwap;
	GLuint program3d;
	GLuint programShadow;
	GLuint programTexture;
	GLuint programDepth;
	GLuint programMixTexture;
	GLuint programWaterTexture;
	


	GLuint modelViewProjectionShadowLoc;
	ProgramLocations * program3dLocs;
	GLuint noiseTexture;
	GLuint screen2dVao;
	GLuint fillAreaVao;
	RenderBuffer shadowFrameBuffer;
	RenderBuffer liquidFrameBuffer;

	int activeTexture = 4; // To avoid rebinding other textures

	float time = 0.0f;

	// UI
	BrushEditor * brushEditor;
	ShadowMapViewer * shadowMapViewer;
	TextureMixerEditor * textureMixerEditor;
	AnimatedTextureEditor * animatedTextureEditor;
	DepthBufferViewer * depthBufferViewer;
	ImageViewer * imageViewer;


public:
	MainApplication() {

	}

	~MainApplication() {

	}






    virtual void setup() {
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		std::vector<GlslInclude> includes;
		includes.push_back(GlslInclude("#include<functions.glsl>" , readFile("shaders/util/functions.glsl")));
		includes.push_back(GlslInclude("#include<perlin.glsl>" , readFile("shaders/util/perlin.glsl")));
		includes.push_back(GlslInclude("#include<functions_fragment.glsl>" , readFile("shaders/util/functions_fragment.glsl")));
		includes.push_back(GlslInclude("#include<structs.glsl>" , readFile("shaders/util/structs.glsl")));
		includes.push_back(GlslInclude("#include<parallax.glsl>" , readFile("shaders/util/parallax.glsl")));

	
		programShadow = createShaderProgram(
			compileShader(replaceIncludes(includes, readFile("shaders/shadow_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/shadow_fragment.glsl")),GL_FRAGMENT_SHADER), 
			0, 
			0
		);
		glUseProgram(programShadow);


		programSwap = createShaderProgram(
			compileShader(replaceIncludes(includes,readFile("shaders/texture/swap_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/swap_fragment.glsl")),GL_FRAGMENT_SHADER), 
			0, 
			0
		);
		glUseProgram(programSwap);

		programTexture = createShaderProgram(
			compileShader(replaceIncludes(includes,readFile("shaders/texture/texture_array_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/texture_array_fragment.glsl")),GL_FRAGMENT_SHADER), 
			0, 
			0
		);
		glUseProgram(programTexture);

		programDepth = createShaderProgram(
			compileShader(replaceIncludes(includes,readFile("shaders/texture/depth_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/depth_fragment.glsl")),GL_FRAGMENT_SHADER), 
			0, 
			0
		);
		glUseProgram(programDepth);

		programMixTexture = createShaderProgram(
			compileShader(replaceIncludes(includes,readFile("shaders/texture/mix_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/mix_fragment.glsl")),GL_FRAGMENT_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/mix_geometry.glsl")),GL_GEOMETRY_SHADER), 
			0
		);
		glUseProgram(programMixTexture);


		programWaterTexture = createShaderProgram(
			compileShader(replaceIncludes(includes,readFile("shaders/texture/water_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/water_fragment.glsl")),GL_FRAGMENT_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/water_geometry.glsl")),GL_GEOMETRY_SHADER), 
			0
		);
		glUseProgram(programWaterTexture);

		program3d = createShaderProgram(
			compileShader(replaceIncludes(includes,readFile("shaders/3d_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/3d_fragment.glsl")),GL_FRAGMENT_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/3d_tessControl.glsl")),GL_TESS_CONTROL_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/3d_tessEvaluation.glsl")),GL_TESS_EVALUATION_SHADER)
		);
		program3dLocs = new ProgramLocations(program3d);
		glUseProgram(program3d);

		// Use the shader program
		screen2dVao = DrawableGeometry::create2DVAO(0,0,200,200);
		fillAreaVao = DrawableGeometry::create2DVAO(-1,1, 1,-1);


		modelViewProjectionShadowLoc = glGetUniformLocation(programShadow, "modelViewProjection");




		shadowFrameBuffer = createDepthFrameBuffer(2048, 2048);
		liquidFrameBuffer = createRenderFrameBuffer(getWidth(), getHeight());


		{
			Texture * t = new Texture(loadTextureArray("textures/grid.png","",""));
			textures.push_back(t);
			brushes.push_back(new Brush(t));
		}
		{
			Texture * t = new Texture(loadTextureArray("textures/lava_color.jpg", "textures/lava_normal.jpg","textures/lava_bump.jpg"));
			textures.push_back(t);
			brushes.push_back(new Brush(t, glm::vec2(1.0), 0.1, 8, 32 ,256, 0.4));
		}
		{
			Texture * t = new Texture(loadTextureArray("textures/grass_color.png", "textures/grass_normal.png", "textures/grass_bump.png"));
			textures.push_back(t);
			brushes.push_back(new Brush(t, glm::vec2(1.0), 0.01, 8, 32 ,32, 0.03));
        }
		{
			Texture * t = new Texture(loadTextureArray("textures/sand_color.jpg", "textures/sand_normal.jpg", "textures/sand_bump.jpg"));
			textures.push_back(t);
			brushes.push_back(new Brush(t, glm::vec2(1.0), 0.05, 8, 32 ,32,0.02));
        }
		{
			Texture * t = new Texture(loadTextureArray("textures/rock_color.png", "textures/rock_normal.png", "textures/rock_bump.png"));
			textures.push_back(t);
			brushes.push_back(new Brush(t, glm::vec2(1.0), 0.1, 8, 32,128, 0.4));
        }
		{
			Texture * t = new Texture(loadTextureArray("textures/snow_color.png", "textures/snow_normal.png", "textures/snow_bump.png"));
			textures.push_back(t);
			brushes.push_back(new Brush(t, glm::vec2(1.0), 0.1, 8, 32, 32 , 0.4));
        }
		{
			Texture * t = new Texture(loadTextureArray("textures/metal_color.png", "textures/metal_normal.png", "textures/metal_bump.png"));
			textures.push_back(t);
			brushes.push_back(new Brush(t, glm::vec2(1.0), 0.1, 8, 64, 32, 0.6 ));
        }
		{
			Texture * t = new Texture(loadTextureArray("textures/dirt_color.png", "textures/dirt_normal.png", "textures/dirt_bump.png"));
			textures.push_back(t);
			brushes.push_back(new Brush(t, glm::vec2(1.0), 0.1, 8, 32 , 256, 0.02));
        }
		{
			Texture * t = new Texture(loadTextureArray("textures/bricks_color.png", "textures/bricks_normal.png", "textures/bricks_bump.png"));
			textures.push_back(t);
			brushes.push_back(new Brush(t, glm::vec2(1.0), 0.01, 8, 32, 256, 0.2 ));
		}
		{
			TextureMixer * tm = new TextureMixer(1024,1024, programMixTexture, &textures);
			tm->mix(2, 3);
			Texture * t = new Texture(tm->getTexture());
			textures.push_back(t);
			brushes.push_back(new Brush(t, glm::vec2(1.0), 0.01, 8, 32, 256, 0.2 ));
			mixers.push_back(tm);
		}
		{
			TextureMixer * tm = new TextureMixer(1024,1024, programMixTexture, &textures);
			tm->mix(2, 5);
			Texture * t = new Texture(tm->getTexture());
			textures.push_back(t);
			brushes.push_back(new Brush(t, glm::vec2(1.0), 0.01, 8, 32, 256, 0.2 ));
			mixers.push_back(tm);
		}

		{
			TextureMixer * tm = new TextureMixer(1024,1024, programMixTexture, &textures);
			tm->mix(4, 2);
			Texture * t = new Texture(tm->getTexture());
			textures.push_back(t);
			brushes.push_back(new Brush(t, glm::vec2(1.0), 0.01, 8, 32, 256, 0.2 ));
			mixers.push_back(tm);
		}
		{
			TextureMixer * tm = new TextureMixer(1024,1024, programMixTexture, &textures);
			tm->mix(4, 5);
			Texture * t = new Texture(tm->getTexture());
			textures.push_back(t);
			brushes.push_back(new Brush(t, glm::vec2(1.0), 0.01, 8, 32, 256, 0.2 ));
			mixers.push_back(tm);
		}
		{
			TextureMixer * tm = new TextureMixer(1024,1024, programMixTexture, &textures);
			tm->mix(4, 3);
			Texture * t = new Texture(tm->getTexture());
			textures.push_back(t);
			brushes.push_back(new Brush(t, glm::vec2(1.0), 0.01, 8, 32, 256, 0.2 ));
			mixers.push_back(tm);
		}
		{
			Texture * t = new Texture(loadTextureArray("textures/ice_color.jpg", "textures/ice_normal.jpg", "textures/ice_bump.jpg"));
			textures.push_back(t);
			brushes.push_back(new Brush(t, glm::vec2(1.0), 0.01, 8, 32, 256, 0.2 ));
		}
		{
			AnimatedTexture * tm = new AnimatedTexture(1024,1024, programWaterTexture, &textures);
			tm->animate(0);
			Texture * t = new Texture(tm->getTexture());
			textures.push_back(t);
			brushes.push_back(new Brush(t, glm::vec2(0.2), 0.02, 8, 32, 10.0, 0.5 ));
			animatedTextures.push_back(tm);
		}


		noiseTexture = loadTextureImage("textures/noise.png");

		activeTexture = Texture::bindTexture(program3d, GL_TEXTURE_2D, activeTexture, "shadowMap", shadowFrameBuffer.depthTexture);
		activeTexture = Texture::bindTexture(program3d, GL_TEXTURE_2D, activeTexture, "noise", noiseTexture);
		
		Texture::bindTextures(program3d, GL_TEXTURE_2D_ARRAY, activeTexture, "textures", &textures);
		Brush::bindBrushes(program3d, &brushes);



        camera.quaternion =   glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1))
   	    					* glm::angleAxis(glm::radians(145.0f), glm::vec3(1, 0, 0))
   	    					* glm::angleAxis(glm::radians(135.0f), glm::vec3(0, 1, 0));  
		camera.position = glm::vec3(48,48,48);

		solidSpace = new Octree(2.0, 5);
		liquidSpace = new Octree(2.0, 5);

		BoundingBox mapBox(glm::vec3(-100,-50,-100), glm::vec3(100,50,100));
		HeightFunction * function = new GradientPerlinSurface(100, 1.0f/128.0f, 0);
		CachedHeightMapSurface * surface = new CachedHeightMapSurface(function, mapBox, solidSpace->minSize);
		HeightMap map(surface, mapBox.getMin(),mapBox.getMax(), solidSpace->minSize);

		solidSpace->add(new HeightMapContainmentHandler(&map, new LandBrush(brushes)));
		//tree->del(new SphereContainmentHandler(BoundingSphere(glm::vec3(00,-30,0),50), textures[7]));
		solidSpace->add(new SphereContainmentHandler(BoundingSphere(glm::vec3(0,50,0),20), new SimpleBrush(textures[6])));
		solidSpace->add(new SphereContainmentHandler(BoundingSphere(glm::vec3(-11,61,11),10), new SimpleBrush(textures[5])));
		solidSpace->del(new SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),10), new SimpleBrush(textures[4])));
		solidSpace->add(new BoxContainmentHandler(BoundingBox(glm::vec3(0,26,0),glm::vec3(24,50,24)),new SimpleBrush(textures[8])));
		solidSpace->del(new SphereContainmentHandler(BoundingSphere(glm::vec3(4,54,-4),8), new SimpleBrush(textures[1])));
		solidSpace->add(new SphereContainmentHandler(BoundingSphere(glm::vec3(11,61,-11),4), new SimpleBrush(textures[3])));


		BoundingBox waterBox(glm::vec3(-100,-50,-100), glm::vec3(100,3,100));
		//liquidSpace->add(new OctreeContainmentHandler(solidSpace, waterBox, new SimpleBrush(textures[6])));
		//BoundingBox waterBox(glm::vec3(50,50,0), glm::vec3(70,70,20));
		liquidSpace->add(new BoxContainmentHandler(waterBox, new SimpleBrush(textures[15])));

		solidTesselator = new Tesselator(solidSpace);
		solidSpace->iterate(solidTesselator);

		liquidTesselator = new Tesselator(liquidSpace);
		liquidSpace->iterate(liquidTesselator);

		solidRenderer = new OctreeRenderer(solidSpace);
		liquidRenderer = new OctreeRenderer(liquidSpace);
		//tesselator->normalize();

		#ifdef DEBUG_GEO
		if(DEBUG_GEO == 0) {
			DebugTesselator * debugTesselator = new DebugTesselator(tree);
			tree->iterate(debugTesselator);
			vaoDebug = new DrawableGeometry(&debugTesselator->chunk);
		}
		#endif

        light.direction = glm::normalize(glm::vec3(-1.0,-1.0,-1.0));
	 
	 	std::cout << "Setup complete!" << std::endl;
		std::cout << "#triangles = " << solidTesselator->triangles << std::endl;
		
		// ImGui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		(void)io;
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(getWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 460");

		brushEditor = new BrushEditor(&camera, &brushes, program3d, programTexture);
		shadowMapViewer = new ShadowMapViewer(shadowFrameBuffer.depthTexture);
		textureMixerEditor = new TextureMixerEditor(&mixers, &textures, programTexture);
		animatedTextureEditor = new AnimatedTextureEditor(&animatedTextures, &textures, programTexture);
		depthBufferViewer = new DepthBufferViewer(programDepth,renderBuffer.depthTexture,512,512);
		imageViewer = new ImageViewer(liquidFrameBuffer.colorTexture, 512,512);
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


		//light.direction = glm::normalize(glm::vec3(glm::sin(time),-1.0,glm::cos(time)));

		float orthoSize = 512.0f;  // Size of the orthographic box

		light.projection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, far);
		light.view = glm::lookAt(lookAtLightPosition - light.direction*dist, lookAtLightPosition, glm::vec3(0,1,0));

		for(int i=0; i<animatedTextures.size() ; ++i) {
			AnimatedTexture * texture = animatedTextures[i];
			texture->animate(time);
		}
    }

	glm::mat4 getCanonicalMVP(glm::mat4 m) {
		return glm::translate(glm::mat4(1.0f), glm::vec3(0.5)) 
						* glm::scale(glm::mat4(1.0f), glm::vec3(0.5)) 
						* m;
	}

    virtual void draw3d() {

		glClearColor (0.0,0.0,0.0,0.0);

		glm::mat4 model = glm::mat4(1.0f); // Identity matrix
		solidRenderer->loaded = 0;
		liquidRenderer->loaded = 0;
		glm::mat4 rotate = glm::mat4_cast(camera.quaternion);
		glm::mat4 translate = glm::translate(glm::mat4(1.0f), -camera.position);
	    camera.view = rotate * translate;
		glm::mat4 mvp = camera.getMVP(model);
		glm::mat4 mlp = light.getMVP(model);
		glm::mat4 ms =  getCanonicalMVP(mlp);


		// ================
		// Shadow component
		// ================
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadowFrameBuffer.frameBuffer);
		glViewport(0, 0, shadowFrameBuffer.width, shadowFrameBuffer.height);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);
		glUseProgram(programShadow);
		glUniformMatrix4fv(modelViewProjectionShadowLoc, 1, GL_FALSE, glm::value_ptr(mlp));

		solidRenderer->mode = GL_TRIANGLES;
		solidRenderer->update(mlp);
		solidSpace->iterate(solidRenderer);


		// ============
		// 3D component
		// ============
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderBuffer.frameBuffer);
		glViewport(0, 0, renderBuffer.width, renderBuffer.height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glPatchParameteri(GL_PATCH_VERTICES, 3); // Define the number of control points per patch
		glEnable(GL_CULL_FACE); // Enable face culling
		glCullFace(GL_BACK); // Or GL_FRONT
		glFrontFace(GL_CCW); // Ensure this matches your vertex data
		glEnable(GL_DEPTH_TEST);
		//glDepthFunc(GL_LESS);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPolygonMode(GL_FRONT, GL_FILL);

		glUseProgram(program3d);
		program3dLocs->update(mvp, model,ms,light.direction, camera.position, time);
		solidRenderer->mode = GL_PATCHES;
		solidRenderer->update(mvp);
		solidSpace->iterate(solidRenderer);




		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, liquidFrameBuffer.frameBuffer);
		glViewport(0, 0, liquidFrameBuffer.width, liquidFrameBuffer.height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUniform1ui(program3dLocs->depthTestEnabledLoc, 1); // Set the sampler uniform
		
		glActiveTexture(GL_TEXTURE0+30); 
		glBindTexture(GL_TEXTURE_2D, renderBuffer.depthTexture);		
		glUniform1i(program3dLocs->depthTextureLoc, 30); // Set the sampler uniform

		glActiveTexture(GL_TEXTURE0+31); 
		glBindTexture(GL_TEXTURE_2D, renderBuffer.colorTexture);		
		glUniform1i(program3dLocs->underTextureLoc, 31); // Set the sampler uniform


		liquidRenderer->mode = GL_PATCHES;
		liquidRenderer->update(mvp);
		liquidSpace->iterate(liquidRenderer);

		if(brushEditor->isOpen()) {
			brushEditor->draw3d();
		}


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


		// ==========
		// Final Pass
		// ==========
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originalFrameBuffer);
		glViewport(0, 0, getWidth(), getHeight());
		glClearColor (0.1,0.1,0.1,1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(programSwap);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glActiveTexture(GL_TEXTURE0); 
		glUniform1i(glGetUniformLocation(programSwap, "textureSampler"), 0); // Set the sampler uniform
		glBindVertexArray(fillAreaVao);

		glBindTexture(GL_TEXTURE_2D, renderBuffer.colorTexture);		
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
		glBindTexture(GL_TEXTURE_2D, liquidFrameBuffer.colorTexture);		
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    }
	bool demo = false;
    bool bSettingsWindow = true;

	virtual void draw2d() {
		// ...
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		if (ImGui::BeginMainMenuBar()) {
			// File Menu
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New")) {
					// Handle "New" action
				}
				if (ImGui::MenuItem("Open", "Ctrl+O")) {
					// Handle "Open" action
				}
				if (ImGui::MenuItem("Save", "Ctrl+S")) {
					// Handle "Save" action
				}
				if (ImGui::MenuItem("Exit")) {
					this->close();
				}
				ImGui::EndMenu();
			}

			// Edit Menu
			if (ImGui::BeginMenu("Tools")) {
				if (ImGui::MenuItem("Animated Textures", "Ctrl+A")) {
					animatedTextureEditor->show();
				}
				if (ImGui::MenuItem("Brush", "Ctrl+B")) {
					brushEditor->show();
				}
				if (ImGui::MenuItem("Depth Buffer Viewer", "Ctrl+B")) {
					depthBufferViewer->show();
				}
				if (ImGui::MenuItem("Image Viewer", "Ctrl+B")) {
					imageViewer->show();
				}
				if (ImGui::MenuItem("Shadow Map Viewer", "Ctrl+D")) {
					shadowMapViewer->show();
				}
				if (ImGui::MenuItem("Texture Mixer", "Ctrl+M")) {
					textureMixerEditor->show();
				}
				if (ImGui::MenuItem("ImGui Demo", "Ctrl+I")) {
					demo = true;
				}
				ImGui::EndMenu();
			}

			// Help Menu
			if (ImGui::BeginMenu("Help")) {
				if (ImGui::MenuItem("About")) {
					// Handle "About" action
				}
				ImGui::EndMenu();
			}

			// End the main menu bar
			ImGui::EndMainMenuBar();
		}


		if(animatedTextureEditor->isOpen()) {
			animatedTextureEditor->draw2d();
		}
		if(brushEditor->isOpen()) {
			brushEditor->draw2d();
		}
		if(shadowMapViewer->isOpen()) {
			shadowMapViewer->draw2d();
		}
		if(textureMixerEditor->isOpen()) {
			textureMixerEditor->draw2d();
		}
		if(depthBufferViewer->isOpen()) {
			depthBufferViewer->draw2d();
		}
		if(imageViewer->isOpen()) {
			imageViewer->draw2d();
		}
		if(demo) {
			ImGui::ShowDemoWindow(&demo);
		}

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originalFrameBuffer);
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

    virtual void clean(){

		// Cleanup and exit
		glDeleteProgram(program3d);
    }

};

int main() {
    MainApplication app;
    app.run();
    return 0;
}

#include "gl/gl.hpp"
#include <math.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/integer.hpp>
#include "DebugTesselator.hpp"
#include "math/math.hpp"
#include "ui/ui.hpp"
#include "tools/tools.hpp"
#include "HeightFunctions.hpp"
#include "Scene.hpp"
//#define DEBUG_GEO 0


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
	GLuint depthTextureLoc;
	GLuint underTextureLoc;
	GLuint cameraPositionLoc;
	GLuint overrideTextureEnabledLoc;
	GLuint shadowMapLoc;
	GLuint noiseLoc;
	GLuint timeLoc;
	GLuint layerLoc;

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
		this->overrideTextureEnabledLoc = glGetUniformLocation(program, "overrideTextureEnabled");
		this->depthTextureLoc = glGetUniformLocation(program, "depthTexture");
		this->underTextureLoc = glGetUniformLocation(program, "underTexture");
		this->layerLoc = glGetUniformLocation(program, "layer");
	}

	void update(glm::mat4 modelViewProjection,glm::mat4 model, glm::mat4 matrixShadow, glm::vec3 lightDirection, glm::vec3 cameraPosition, float time, Settings * settings) {
		glUniformMatrix4fv(modelViewProjectionLoc, 1, GL_FALSE, glm::value_ptr(modelViewProjection));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(matrixShadowLoc, 1, GL_FALSE, glm::value_ptr(matrixShadow  ));
		glUniform3fv(lightDirectionLoc, 1, glm::value_ptr(lightDirection));
		glUniform3fv(cameraPositionLoc, 1, glm::value_ptr(cameraPosition));
		glUniform1f(timeLoc, time);
		glUniform1ui(lightEnabledLoc, settings->lightEnabled);
		glUniform1ui(triplanarEnabledLoc, 1);
		glUniform1ui(parallaxEnabledLoc, settings->parallaxEnabled);
		glUniform1ui(debugEnabledLoc,settings->debugEnabled);
		glUniform1ui(shadowEnabledLoc, settings->shadowEnabled);
	}

	void updateWireframe() {
		glUniform1ui(lightEnabledLoc, 0);
		glUniform1ui(parallaxEnabledLoc, 0);
		glUniform1ui(triplanarEnabledLoc, 0);
		glUniform1ui(debugEnabledLoc, 1);

	}
};

class MainApplication : public LithosApplication {
	std::vector<Texture*> textures;
	std::vector<Brush*> brushes;
	std::vector<Brush*> vegetationBrushes;
	std::vector<AtlasTexture*> atlasTextures;
	std::vector<AtlasDrawer*> atlasDrawers;
	std::vector<TextureMixer*> mixers;
	std::vector<AnimatedTexture*> animatedTextures;

	Scene * mainScene;


	Settings * settings = new Settings();
	#ifdef DEBUG_GEO
	DrawableGeometry * vaoDebug;
	#endif

	GLuint programSwap;
	GLuint program3d;
	GLuint programShadow;
	GLuint programAtlas;
	GLuint programTexture;
	GLuint programDepth;
	GLuint programMixTexture;
	GLuint programWaterTexture;


	GLuint modelViewProjectionShadowLoc;
	ProgramLocations * program3dLocs;
	GLuint noiseTexture;

	RenderBuffer depthFrameBuffer;
	RenderBuffer renderBuffer;
	RenderBuffer solidBuffer;
	RenderBuffer shadowFrameBuffer;

	int activeTexture = 5; // To avoid rebinding other textures

	
	float time = 0.0f;

	// UI
	BrushEditor * brushEditor;
	AtlasPainter * atlasPainter;
	AtlasViewer * atlasViewer;
	ShadowMapViewer * shadowMapViewer;
	TextureMixerEditor * textureMixerEditor;
	AnimatedTextureEditor * animatedTextureEditor;
	DepthBufferViewer * depthBufferViewer;
	SettingsEditor * settingsEditor;
	TextureViewer * textureViewer;

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

		programShadow = createShaderProgram({
			compileShader(replaceIncludes(includes, readFile("shaders/shadow_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/shadow_fragment.glsl")),GL_FRAGMENT_SHADER)
		});
		glUseProgram(programShadow);

		programAtlas = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/texture/atlas_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/atlas_geometry.glsl")),GL_GEOMETRY_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/atlas_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});
		glUseProgram(programAtlas);

		programSwap = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/texture/swap_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/swap_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});
		glUseProgram(programSwap);

		programTexture = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/texture/texture_array_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/texture_array_fragment.glsl")),GL_FRAGMENT_SHADER)
		});
		glUseProgram(programTexture);

		programDepth = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/texture/depth_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/depth_fragment.glsl")),GL_FRAGMENT_SHADER)
		});
		glUseProgram(programDepth);

		programMixTexture = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/texture/mix_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/mix_geometry.glsl")),GL_GEOMETRY_SHADER),
			compileShader(replaceIncludes(includes,readFile("shaders/texture/mix_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});
		glUseProgram(programMixTexture);

		programWaterTexture = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/texture/water_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/water_geometry.glsl")),GL_GEOMETRY_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/water_fragment.glsl")),GL_FRAGMENT_SHADER)
		});
		glUseProgram(programWaterTexture);

		program3d = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/3d_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/3d_tessControl.glsl")),GL_TESS_CONTROL_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/3d_tessEvaluation.glsl")),GL_TESS_EVALUATION_SHADER),
			compileShader(replaceIncludes(includes,readFile("shaders/3d_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});
		program3dLocs = new ProgramLocations(program3d);
		glUseProgram(program3d);

		modelViewProjectionShadowLoc = glGetUniformLocation(programShadow, "modelViewProjection");

		renderBuffer = createRenderFrameBuffer(getWidth(), getHeight());
		solidBuffer = createRenderFrameBuffer(getWidth(), getHeight());
		depthFrameBuffer = createDepthFrameBuffer(getWidth(), getHeight());
		shadowFrameBuffer = createDepthFrameBuffer(2048, 2048);

		{
			AnimatedTexture * tm = new AnimatedTexture(1024,1024, programWaterTexture);
			tm->animate(0);
			Texture * t = new Texture(tm->getTexture());
			brushes.push_back(new Brush(textures.size(), glm::vec2(0.2), 0.02, 8, 32, 16,4, 10.0, 0.5 , 1.33));
			animatedTextures.push_back(tm);
			textures.push_back(t);
		}
		{
			Texture * t = new Texture(loadTextureArray({"textures/lava_color.jpg", "textures/lava_normal.jpg","textures/lava_bump.jpg"}));
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.1, 8, 32, 16,4 ,256, 0.4, 0.0));
			textures.push_back(t);
		}
		{
			Texture * t = new Texture(loadTextureArray({"textures/grass_color.png", "textures/grass_normal.png", "textures/grass_bump.png"}));
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.01, 2, 8, 8,4 ,32, 0.03, 0.0));
			textures.push_back(t);
        }
		{
			Texture * t = new Texture(loadTextureArray({"textures/sand_color.jpg", "textures/sand_normal.jpg", "textures/sand_bump.jpg"}));
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.05, 8, 32, 16,4 ,32,0.02, 0.0));
			textures.push_back(t);
        }
		{
			Texture * t = new Texture(loadTextureArray({"textures/rock_color.png", "textures/rock_normal.png", "textures/rock_bump.png"}));
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.1, 8, 32, 16,4,128, 0.4, 0.0));
			textures.push_back(t);
        }
		{
			Texture * t = new Texture(loadTextureArray({"textures/snow_color.png", "textures/snow_normal.png", "textures/snow_bump.png"}));
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.1, 8, 32, 16,4, 32 , 0.4, 0.0));
			textures.push_back(t);
        }
		{
			Texture * t = new Texture(loadTextureArray({"textures/metal_color.png", "textures/metal_normal.png", "textures/metal_bump.png"}));
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.1, 8, 64, 64,4, 32, 0.6 , 0.0));
			textures.push_back(t);
        }
		{
			Texture * t = new Texture(loadTextureArray({"textures/dirt_color.png", "textures/dirt_normal.png", "textures/dirt_bump.png"}));
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.1, 8, 32, 16,4 , 256, 0.02, 0.0));
			textures.push_back(t);
        }
		{
			Texture * t = new Texture(loadTextureArray({"textures/bricks_color.png", "textures/bricks_normal.png", "textures/bricks_bump.png"}));
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.01, 8, 32, 16,4, 256, 0.2 , 0.0));
			textures.push_back(t);
		}
		{
			TextureMixer * tm = new TextureMixer(1024,1024, programMixTexture, &textures);
			tm->mix(2, 3);
			Texture * t = new Texture(tm->getTexture());
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.01, 8, 32, 16,4, 256, 0.2 , 0.0));
			mixers.push_back(tm);
			textures.push_back(t);
		}
		{
			TextureMixer * tm = new TextureMixer(1024,1024, programMixTexture, &textures);
			tm->mix(2, 5);
			Texture * t = new Texture(tm->getTexture());
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.01, 8, 32, 16,4, 256, 0.2 , 0.0));
			mixers.push_back(tm);
			textures.push_back(t);
		}
		{
			TextureMixer * tm = new TextureMixer(1024,1024, programMixTexture, &textures);
			tm->mix(4, 2);
			Texture * t = new Texture(tm->getTexture());
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.01, 8, 32, 16,4, 256, 0.2 , 0.0));
			mixers.push_back(tm);
			textures.push_back(t);
		}
		{
			TextureMixer * tm = new TextureMixer(1024,1024, programMixTexture, &textures);
			tm->mix(4, 5);
			Texture * t = new Texture(tm->getTexture());
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.01, 8, 32, 16,4, 256, 0.2 , 0.0));
			mixers.push_back(tm);
			textures.push_back(t);
		}
		{
			TextureMixer * tm = new TextureMixer(1024,1024, programMixTexture, &textures);
			tm->mix(4, 3);
			Texture * t = new Texture(tm->getTexture());
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.01, 8, 32, 16,4, 256, 0.2 , 0.0));
			mixers.push_back(tm);
			textures.push_back(t);
		}
		{
			Texture * t = new Texture(loadTextureArray({"textures/soft_sand_color.jpg", "textures/soft_sand_normal.jpg", "textures/soft_sand_bump.jpg"}));
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.01, 4, 16, 8,4, 256, 0.2 , 0.0));
			textures.push_back(t);
		}
		{
			Texture * t = new Texture(loadTextureArray({"textures/forest_color.png", "textures/forest_normal.png","textures/forest_bump.png"}));
			brushes.push_back(new Brush(textures.size()));
			textures.push_back(t);
		}
		{
			AtlasTexture * at = new AtlasTexture(loadTextureArray({"textures/vegetation/foliage_color.png", "textures/vegetation/foliage_normal.png", "textures/vegetation/foliage_bump.png", "textures/vegetation/foliage_opacity.png"}));
			at->tiles.push_back(Tile(glm::vec2(1.0),glm::vec2(0.0)));
			at->tiles.push_back(Tile(glm::vec2(0.15, 1.0),glm::vec2(0.0, 0.0)));
			at->tiles.push_back(Tile(glm::vec2(0.15, 0.5),glm::vec2(0.15, 0.0)));
			at->tiles.push_back(Tile(glm::vec2(0.15, 0.5),glm::vec2(0.15, 0.5)));
			at->tiles.push_back(Tile(glm::vec2(0.15, 0.5),glm::vec2(0.30, 0.0)));
			at->tiles.push_back(Tile(glm::vec2(0.15, 0.5),glm::vec2(0.30, 0.5)));
			at->tiles.push_back(Tile(glm::vec2(0.15, 0.5),glm::vec2(0.45, 0.0)));
			at->tiles.push_back(Tile(glm::vec2(0.15, 0.5),glm::vec2(0.45, 0.5)));
			at->tiles.push_back(Tile(glm::vec2(0.4, 0.5),glm::vec2(0.6, 0.0)));
			at->tiles.push_back(Tile(glm::vec2(0.4, 0.5),glm::vec2(0.6, 0.5)));

			atlasTextures.push_back(at);
			//brushes.push_back(new Brush(at, glm::vec2(0.2), 0.02, 8, 32, 16,4, 10.0, 0.5 , 1.33));

			AtlasDrawer * ad = new AtlasDrawer(programAtlas, 256, 256, &atlasTextures);
			std::vector<TileDraw> draws;
			draws.push_back(TileDraw(0,glm::vec2(1), glm::vec2(0.5), glm::vec2(0.5), 0.5));
			ad->draw(0, draws);

			atlasDrawers.push_back(ad);
		}
		{
			AtlasTexture * at = new AtlasTexture(loadTextureArray({"textures/vegetation/grass_color.jpg", "textures/vegetation/grass_normal.jpg", "textures/vegetation/grass_bump.jpg", "textures/vegetation/grass_opacity.jpg"}));
			at->tiles.push_back(Tile(glm::vec2(1.0),glm::vec2(0.0)));
			
			atlasTextures.push_back(at);

			AtlasDrawer * ad = new AtlasDrawer(programAtlas, 256, 256, &atlasTextures);
			std::vector<TileDraw> draws;
			draws.push_back(TileDraw(0,glm::vec2(1), glm::vec2(0.5), glm::vec2(0.5), 0.0));
			ad->draw(1, draws);

			atlasDrawers.push_back(ad);
		}



		noiseTexture = loadTextureImage("textures/noise.png");
		activeTexture = Texture::bindTexture(program3d, GL_TEXTURE_2D, activeTexture, "depthTexture", depthFrameBuffer.depthTexture);
		activeTexture = Texture::bindTexture(program3d, GL_TEXTURE_2D, activeTexture, "underTexture", solidBuffer.colorTexture);
		activeTexture = Texture::bindTexture(program3d, GL_TEXTURE_2D, activeTexture, "shadowMap", shadowFrameBuffer.depthTexture);
		activeTexture = Texture::bindTexture(program3d, GL_TEXTURE_2D, activeTexture, "noise", noiseTexture);

		Texture::bindTextures(program3d, GL_TEXTURE_2D_ARRAY, activeTexture, "textures", &textures);

		Brush::bindBrushes(program3d, &brushes);

		mainScene = new Scene();
		mainScene->setup();
		mainScene->load();
	
		//tesselator->normalize();

		#ifdef DEBUG_GEO
		DebugTesselator * debugTesselator = new DebugTesselator(liquidSpace);
		liquidSpace->iterate(debugTesselator);
		vaoDebug = new DrawableGeometry(&debugTesselator->chunk);
		#endif

		atlasPainter = new AtlasPainter(&atlasTextures, &atlasDrawers, programAtlas, programTexture, 256,256);
		atlasViewer = new AtlasViewer(&atlasTextures, programAtlas, programTexture, 256,256);
		brushEditor = new BrushEditor(&mainScene->camera, &brushes, &textures, program3d, programTexture);
		shadowMapViewer = new ShadowMapViewer(shadowFrameBuffer.depthTexture);
		textureMixerEditor = new TextureMixerEditor(&mixers, &textures, programTexture);
		animatedTextureEditor = new AnimatedTextureEditor(&animatedTextures, &textures, programTexture, 256,256);
		depthBufferViewer = new DepthBufferViewer(programDepth,renderBuffer.depthTexture,256,256);
		settingsEditor = new SettingsEditor(settings);
		textureViewer = new TextureViewer(&textures, programTexture);

		// ImGui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		(void)io;
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(getWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 460");

	}

    virtual void update(float deltaTime){
		time += deltaTime;
	   	if (getKeyboardStatus(GLFW_KEY_ESCAPE) != GLFW_RELEASE) {
		   	close();
		}

        mainScene->camera.projection = glm::perspective(glm::radians(45.0f), getWidth() / (float) getHeight(), 0.1f, 512.0f);
	//    camera.projection[1][1] *= -1;
	 //   modelMatrix = glm::rotate(modelMatrix, deltaTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	   	float rsense = 0.01;

	   	if (getKeyboardStatus(GLFW_KEY_W) != GLFW_RELEASE) {
	   	   	mainScene->camera.quaternion = glm::angleAxis(+rsense, glm::vec3(1,0,0))*mainScene->camera.quaternion;
		}
	   	if (getKeyboardStatus(GLFW_KEY_S) != GLFW_RELEASE) {
	   	   	mainScene->camera.quaternion = glm::angleAxis(-rsense, glm::vec3(1,0,0))*mainScene->camera.quaternion;
		}
   		if (getKeyboardStatus(GLFW_KEY_A) != GLFW_RELEASE) {
	   	   	mainScene->camera.quaternion = glm::angleAxis(-rsense, glm::vec3(0,1,0))*mainScene->camera.quaternion;
		}
	   	if (getKeyboardStatus(GLFW_KEY_D) != GLFW_RELEASE) {
	   	   	mainScene->camera.quaternion = glm::angleAxis(+rsense, glm::vec3(0,1,0))*mainScene->camera.quaternion;
		}
		if (getKeyboardStatus(GLFW_KEY_Q) != GLFW_RELEASE) {
	   	   	mainScene->camera.quaternion = glm::angleAxis(+rsense, glm::vec3(0,0,1))*mainScene->camera.quaternion;
		}
	   	if (getKeyboardStatus(GLFW_KEY_E) != GLFW_RELEASE) {
	   	   	mainScene->camera.quaternion = glm::angleAxis(-rsense, glm::vec3(0,0,1))*mainScene->camera.quaternion;
		}

		glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f)*mainScene->camera.quaternion;
		glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f)*mainScene->camera.quaternion;
		glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f)*mainScene->camera.quaternion;

	   	float tsense = deltaTime*10;
	   	if (getKeyboardStatus(GLFW_KEY_UP) != GLFW_RELEASE) {
	   		mainScene->camera.position -= zAxis*tsense;
		}
	   	if (getKeyboardStatus(GLFW_KEY_DOWN) != GLFW_RELEASE) {
	   		mainScene->camera.position += zAxis*tsense;
		}
	   	if (getKeyboardStatus(GLFW_KEY_RIGHT) != GLFW_RELEASE) {
	   		mainScene->camera.position += xAxis*tsense;
		}
   		if (getKeyboardStatus(GLFW_KEY_LEFT) != GLFW_RELEASE) {
	   		mainScene->camera.position -= xAxis*tsense;
		}

		mainScene->camera.quaternion = glm::normalize(mainScene->camera.quaternion);
		glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f)*mainScene->camera.quaternion;
	
		float far = 512.0f;
		float dist = 32.0f;
	   	glm::vec3 lookAtLightPosition = glm::round(mainScene->camera.position/16.0f)*16.0f; // + cameraDirection*far*0.5f;

		//light.direction = glm::normalize(glm::vec3(glm::sin(time),-1.0,glm::cos(time)));

		float orthoSize = 512.0f;  // Size of the orthographic box
		mainScene->light.projection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, far);
		mainScene->light.view = glm::lookAt(lookAtLightPosition - mainScene->light.direction*dist, lookAtLightPosition, glm::vec3(0,1,0));

		for(int i=0; i<animatedTextures.size() ; ++i) {
			AnimatedTexture * texture = animatedTextures[i];
			texture->animate(time);
		}
    }



    virtual void draw3d() {
		glClearColor (0.1,0.1,0.1,1.0);
		glm::mat4 model = glm::mat4(1.0f); // Identity matrix

		glm::mat4 rotate = glm::mat4_cast(mainScene->camera.quaternion);
		glm::mat4 translate = glm::translate(glm::mat4(1.0f), -mainScene->camera.position);
	    mainScene->camera.view = rotate * translate;
		glm::mat4 mvp = mainScene->camera.getMVP(model);
		glm::mat4 mlp = mainScene->light.getMVP(model);
		glm::mat4 ms =  Math::getCanonicalMVP(mlp);

		mainScene->update3d(mvp, mlp);

		// ================
		// Shadow component
		// ================
		if(settings->shadowEnabled) {
			glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffer.frameBuffer);
			glViewport(0, 0, shadowFrameBuffer.width, shadowFrameBuffer.height);
			glEnable(GL_DEPTH_TEST);
			glClear(GL_DEPTH_BUFFER_BIT);
			glUseProgram(programShadow);
			glUniformMatrix4fv(modelViewProjectionShadowLoc, 1, GL_FALSE, glm::value_ptr(mlp));
			mainScene->draw3dShadow();
		}

		// ============
		// 3D component
		// ============
		glPatchParameteri(GL_PATCH_VERTICES, 3); // Define the number of control points per patch
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // Or GL_FRONT
		glFrontFace(GL_CCW); // Ensure this matches your vertex data
		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPolygonMode(GL_FRONT, GL_FILL);


		glUseProgram(program3d);
		program3dLocs->update(mvp, model,ms,mainScene->light.direction, mainScene->camera.position, time, settings);
			

		// =================
		// First Pass: Depth
		// =================
		glBindFramebuffer(GL_FRAMEBUFFER, depthFrameBuffer.frameBuffer);
		glViewport(0, 0, depthFrameBuffer.width, depthFrameBuffer.height);
		glClear(GL_DEPTH_BUFFER_BIT);
		glUniform1i(program3dLocs->layerLoc, 0);
		mainScene->draw3dSolid();


		// ==================
		// Second Pass: Solid
		//===================
		glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer.frameBuffer);
		glViewport(0, 0, renderBuffer.width, renderBuffer.height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUniform1i(program3dLocs->layerLoc, 1); 

		if(settings->wireFrameEnabled) {
			program3dLocs->updateWireframe();

			glPolygonMode(GL_FRONT, GL_LINE);
			glLineWidth(2.0);
			glPointSize(4.0);	

			mainScene->draw3dSolid();
			mainScene->draw3dLiquid();

			mainScene->drawVegetation();
			//glUseProgram(program3d);

		} else {
			glPolygonMode(GL_FRONT, GL_FILL);
	
			mainScene->draw3dSolid();

			mainScene->drawVegetation();

			glUseProgram(program3d);
			program3dLocs->update(mvp, model,ms,mainScene->light.direction, mainScene->camera.position, time, settings);

			// Bind the source framebuffer (FBO you rendered to)
			glBindFramebuffer(GL_READ_FRAMEBUFFER, renderBuffer.frameBuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, solidBuffer.frameBuffer);
			glBlitFramebuffer(
				0, 0, renderBuffer.width, renderBuffer.height, // Source rectangle (x0, y0, x1, y1)
				0, 0, solidBuffer.width, solidBuffer.height, 
				GL_COLOR_BUFFER_BIT,  
				GL_NEAREST           
			);

			glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer.frameBuffer);
			glViewport(0, 0, renderBuffer.width, renderBuffer.height);
			
			mainScene->draw3dLiquid();
		}

		brushEditor->draw3dIfOpen();
		
		glPolygonMode(GL_FRONT, GL_FILL);

		#ifdef DEBUG_GEO
	
		glUniform1ui(program3dLocs->lightEnabledLoc, 0);
		glUniform1ui(program3dLocs->parallaxEnabledLoc, 0);
    	glUniform1ui(program3dLocs->triplanarEnabledLoc, 0);
	
		glDisable(GL_CULL_FACE); 
		vaoDebug->draw(GL_PATCHES);
			
		glPolygonMode(GL_FRONT, GL_FILL);
		#endif

		// ==========
		// Final Pass
		// ==========
		glBindFramebuffer(GL_READ_FRAMEBUFFER, renderBuffer.frameBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originalFrameBuffer);
		glBlitFramebuffer(
			0, 0, renderBuffer.width, renderBuffer.height, // Source rectangle (x0, y0, x1, y1)
			0, 0, getWidth(), getHeight(), 
			GL_COLOR_BUFFER_BIT,  
			GL_NEAREST           
		);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
					mainScene->create(textures, brushes);
				}
				if (ImGui::MenuItem("Open", "Ctrl+O")) {
					mainScene->load();
				}
				if (ImGui::MenuItem("Save", "Ctrl+S")) {
					mainScene->save();
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
				if (ImGui::MenuItem("Atlas Painter", "Ctrl+B")) {
					atlasPainter->show();
				}
				if (ImGui::MenuItem("Atlas Viewer", "Ctrl+B")) {
					atlasViewer->show();
				}		
				if (ImGui::MenuItem("Depth Buffer Viewer", "Ctrl+B")) {
					depthBufferViewer->show();
				}
				if (ImGui::MenuItem("Shadow Map Viewer", "Ctrl+D")) {
					shadowMapViewer->show();
				}
				if (ImGui::MenuItem("Texture Mixer", "Ctrl+M")) {
					textureMixerEditor->show();
				}
				if (ImGui::MenuItem("Texture Viewer", "Ctrl+M")) {
					textureViewer->show();
				}
				if (ImGui::MenuItem("Settings", "Ctrl+S")) {
					settingsEditor->show();
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

			ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Always);
			ImGui::SetNextWindowBgAlpha(0.35f); // Set background transparency

			ImGui::Begin("Overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
										ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | 
										ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | 
										ImGuiWindowFlags_NoNav);
			
			
			#ifdef GL_ATI_meminfo
				GLint vram_usage[4] = {0};
				glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, vram_usage);

				ImGui::Text("Available VRAM: %d MB", vram_usage[0] / 1024);
			#endif
			
			ImGui::Text("%d FPS", framesPerSecond);
			ImGui::Text("%d solid triangles", mainScene->solidTrianglesCount);
			ImGui::Text("%d liquid triangles", mainScene->liquidTrianglesCount);
			ImGui::Text("%d shadow triangles", mainScene->shadowTrianglesCount);
			ImGui::End();

		}

		animatedTextureEditor->draw2dIfOpen();
		brushEditor->draw2dIfOpen();
		shadowMapViewer->draw2dIfOpen();
		textureMixerEditor->draw2dIfOpen();
		depthBufferViewer->draw2dIfOpen();
		settingsEditor->draw2dIfOpen();
		atlasViewer->draw2dIfOpen();
		atlasPainter->draw2dIfOpen();
		textureViewer->draw2dIfOpen();
		

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

#include "gl/gl.hpp"
#include <math.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/integer.hpp>
#include "math/math.hpp"
#include "ui/ui.hpp"
#include "tools/tools.hpp"
#include "HeightFunctions.hpp"
#include "Scene.hpp"



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
	for(GlslInclude include : includes) {
		code = replace(code, include.line, include.code);
	}
	return code;
}



class MainApplication : public LithosApplication {
	std::vector<Texture*> textures;
	std::vector<Texture*> billboardTextures;
	std::vector<Brush*> brushes;
	std::vector<Brush*> billboardBrushes;
	std::vector<AtlasTexture*> atlasTextures;
	std::vector<AtlasDrawer*> atlasDrawers;
	std::vector<TextureMixer*> mixers;
	std::vector<AnimatedTexture*> animatedTextures;

	Scene * mainScene;
	Settings * settings = new Settings();
	glm::mat4 worldModel = glm::mat4(1.0f); // Identity matrix

	Camera camera;
	DirectionalLight light;

	GLuint programSwap;
	GLuint program3d;
	GLuint programBillboard;
	GLuint programAtlas;
	GLuint programTexture;
	GLuint programDepth;
	GLuint programMixTexture;
	GLuint programWaterTexture;

	ProgramData * program3dData;
	ProgramData * programBillboardData;

	UniformBlock viewerBlock;


	GLuint noiseTexture;

	RenderBuffer depthFrameBuffer;
	RenderBuffer renderBuffer;
	RenderBuffer solidBuffer;
	std::vector<std::pair<RenderBuffer, int>> shadowFrameBuffers;

	int activeTexture = 5; // To avoid rebinding other textures

	
	float time = 0.0f;

	// UI
	UniformBlockViewer * uniformBlockViewer;
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
		std::vector<GlslInclude> includes;
		includes.push_back(GlslInclude("#include<functions.glsl>" , readFile("shaders/util/functions.glsl")));
		includes.push_back(GlslInclude("#include<perlin.glsl>" , readFile("shaders/util/perlin.glsl")));
		includes.push_back(GlslInclude("#include<functions_fragment.glsl>" , readFile("shaders/util/functions_fragment.glsl")));
		includes.push_back(GlslInclude("#include<structs.glsl>" , readFile("shaders/util/structs.glsl")));
		includes.push_back(GlslInclude("#include<parallax.glsl>" , readFile("shaders/util/parallax.glsl")));
		includes.push_back(GlslInclude("#include<depth.glsl>" , readFile("shaders/util/depth.glsl")));

		programAtlas = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/texture/atlas_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/atlas_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});

		programSwap = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/texture/swap_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/swap_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});

		programTexture = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/texture/texture_array_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/texture_array_fragment.glsl")),GL_FRAGMENT_SHADER)
		});

		programDepth = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/texture/depth_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/depth_fragment.glsl")),GL_FRAGMENT_SHADER)
		});

		programMixTexture = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/texture/mix_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/mix_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});

		programWaterTexture = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/texture/water_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/water_fragment.glsl")),GL_FRAGMENT_SHADER)
		});



		program3d = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/3d_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/3d_tessControl.glsl")),GL_TESS_CONTROL_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/3d_tessEvaluation.glsl")),GL_TESS_EVALUATION_SHADER),
			compileShader(replaceIncludes(includes,readFile("shaders/3d_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});
		program3dData = new ProgramData();

		programBillboard = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/3d_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/3d_tessControl.glsl")),GL_TESS_CONTROL_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/3d_tessEvaluation.glsl")),GL_TESS_EVALUATION_SHADER),
			compileShader(replaceIncludes(includes,readFile("shaders/3d_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});
		programBillboardData = new ProgramData();


		renderBuffer = createRenderFrameBuffer(getWidth(), getHeight());
		solidBuffer = createRenderFrameBuffer(getWidth(), getHeight());
		depthFrameBuffer = createDepthFrameBuffer(getWidth(), getHeight());
		shadowFrameBuffers.push_back(std::pair(createDepthFrameBuffer(1024, 1024), 32));
		shadowFrameBuffers.push_back(std::pair(createDepthFrameBuffer(1024, 1024), 128));
		shadowFrameBuffers.push_back(std::pair(createDepthFrameBuffer(1024, 1024), 512));

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
			Texture * t = new Texture(loadTextureArray({"textures/grass_color.jpg", "textures/grass_normal.jpg", "textures/grass_bump.jpg"}));
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.01, 2, 8, 8,4 ,32, 0.03, 0.0));
			textures.push_back(t);
        }
		{
			Texture * t = new Texture(loadTextureArray({"textures/sand_color.jpg", "textures/sand_normal.jpg", "textures/sand_bump.jpg"}));
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.05, 8, 32, 16,4 ,32,0.02, 0.0));
			textures.push_back(t);
        }
		{
			Texture * t = new Texture(loadTextureArray({"textures/rock_color.jpg", "textures/rock_normal.jpg", "textures/rock_bump.jpg"}));
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.1, 8, 32, 16,4,128, 0.4, 0.0));
			textures.push_back(t);
        }
		{
			Texture * t = new Texture(loadTextureArray({"textures/snow_color.jpg", "textures/snow_normal.jpg", "textures/snow_bump.jpg"}));
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.1, 8, 32, 16,4, 32 , 0.4, 0.0));
			textures.push_back(t);
        }
		{
			Texture * t = new Texture(loadTextureArray({"textures/metal_color.jpg", "textures/metal_normal.jpg", "textures/metal_bump.jpg"}));
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.1, 8, 64, 64,4, 32, 0.6 , 0.0));
			textures.push_back(t);
        }
		{
			Texture * t = new Texture(loadTextureArray({"textures/dirt_color.jpg", "textures/dirt_normal.jpg", "textures/dirt_bump.jpg"}));
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.1, 8, 32, 16,4 , 256, 0.02, 0.0));
			textures.push_back(t);
        }
		{
			Texture * t = new Texture(loadTextureArray({"textures/bricks_color.jpg", "textures/bricks_normal.jpg", "textures/bricks_bump.jpg"}));
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
			Texture * t = new Texture(loadTextureArray({"textures/forest_color.jpg", "textures/forest_normal.jpg","textures/forest_bump.jpg"}));
			brushes.push_back(new Brush(textures.size(), glm::vec2(1.0), 0.01, 4, 16, 8,4, 256, 0.2 , 0.0));
			textures.push_back(t);
		}
		{
			AtlasTexture * at = new AtlasTexture(loadTextureArray({"textures/vegetation/foliage_color.jpg", "textures/vegetation/foliage_normal.jpg", "textures/vegetation/foliage_bump.jpg", "textures/vegetation/foliage_opacity.jpg"}));
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
			billboardBrushes.push_back(new Brush(billboardTextures.size()));
			billboardTextures.push_back(new Texture(ad->getTexture()));
		}
		{
			AtlasTexture * at = new AtlasTexture(loadTextureArray({"textures/vegetation/grass_color.jpg", "textures/vegetation/grass_normal.jpg", "textures/vegetation/grass_bump.jpg", "textures/vegetation/grass_opacity.jpg"}));
			at->tiles.push_back(Tile(glm::vec2(1.0),glm::vec2(0.0)));
			
			atlasTextures.push_back(at);

			AtlasDrawer * ad = new AtlasDrawer(programAtlas, 1024, 1024, &atlasTextures);
			std::vector<TileDraw> draws;
			draws.push_back(TileDraw(0,glm::vec2(1), glm::vec2(0.5), glm::vec2(0.5), 0.0));
			ad->draw(1, draws);

			atlasDrawers.push_back(ad);
			billboardBrushes.push_back(new Brush(billboardTextures.size()));
			billboardTextures.push_back(new Texture(ad->getTexture()));
		}

		noiseTexture = loadTextureImage("textures/noise.jpg");

		Texture::bindTexture(program3d, GL_TEXTURE_2D, activeTexture, glGetUniformLocation(program3d, "noise"), noiseTexture);
		activeTexture = Texture::bindTexture(programBillboard, GL_TEXTURE_2D, activeTexture, glGetUniformLocation(programBillboard, "noise"), noiseTexture);

		Texture::bindTexture(program3d, GL_TEXTURE_2D, activeTexture,  glGetUniformLocation(program3d, "depthTexture"), depthFrameBuffer.depthTexture);
		activeTexture = Texture::bindTexture(programBillboard, GL_TEXTURE_2D, activeTexture,  glGetUniformLocation(programBillboard, "depthTexture"), depthFrameBuffer.depthTexture);

		Texture::bindTexture(program3d, GL_TEXTURE_2D, activeTexture, glGetUniformLocation(program3d, "underTexture"), solidBuffer.colorTexture);
		activeTexture = Texture::bindTexture(programBillboard, GL_TEXTURE_2D, activeTexture, glGetUniformLocation(programBillboard, "underTexture"), solidBuffer.colorTexture);

		for(int i=0; i < shadowFrameBuffers.size(); ++i) {
			std::string shadowMapName = "shadowMap["+ std::to_string(i) +"]";
			Texture::bindTexture(program3d, GL_TEXTURE_2D, activeTexture, glGetUniformLocation(program3d, shadowMapName.c_str()), shadowFrameBuffers[i].first.depthTexture);
			activeTexture = Texture::bindTexture(programBillboard, GL_TEXTURE_2D, activeTexture, glGetUniformLocation(programBillboard, shadowMapName.c_str()), shadowFrameBuffers[i].first.depthTexture);
		}
		activeTexture = Texture::bindTextures(programBillboard, GL_TEXTURE_2D_ARRAY, activeTexture, "textures", &billboardTextures);
		activeTexture = Texture::bindTextures(program3d, GL_TEXTURE_2D_ARRAY, activeTexture, "textures", &textures);

		Brush::bindBrushes(program3d,"brushes", "brushTextures", &brushes);
		Brush::bindBrushes(programBillboard, "brushes", "brushTextures", &billboardBrushes);

		mainScene = new Scene();
		mainScene->setup();
		mainScene->load();
	
		
		camera.quaternion =   glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1))
		* glm::angleAxis(glm::radians(145.0f), glm::vec3(1, 0, 0))
		* glm::angleAxis(glm::radians(135.0f), glm::vec3(0, 1, 0));  
		camera.position = glm::vec3(48,48,48);
        light.direction = glm::normalize(glm::vec3(-1.0,-1.0,-1.0));

		//tesselator->normalize();
		uniformBlockViewer = new UniformBlockViewer(&viewerBlock);
		atlasPainter = new AtlasPainter(&atlasTextures, &atlasDrawers, programAtlas, programTexture, 256,256);
		atlasViewer = new AtlasViewer(&atlasTextures, programAtlas, programTexture, 256,256);
		brushEditor = new BrushEditor(&camera, &brushes, &textures, program3d, programTexture);
		shadowMapViewer = new ShadowMapViewer(&shadowFrameBuffers, 512, 512);
		textureMixerEditor = new TextureMixerEditor(&mixers, &textures, programTexture);
		animatedTextureEditor = new AnimatedTextureEditor(&animatedTextures, &textures, programTexture, 256,256);
		depthBufferViewer = new DepthBufferViewer(programDepth,depthFrameBuffer.depthTexture,512,512);
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

	

		for(int i=0; i<animatedTextures.size() ; ++i) {
			AnimatedTexture * texture = animatedTextures[i];
			texture->animate(time);
		}
    }

	

    virtual void draw3d() {
		float far = 512.0f;
		float near = 0.1f;

		glm::mat4 rotate = glm::mat4_cast(camera.quaternion);
		glm::mat4 translate = glm::translate(glm::mat4(1.0f), -camera.position);
	   
		camera.projection = glm::perspective(glm::radians(45.0f), getWidth() / (float) getHeight(), near, far);
		camera.view = rotate * translate;
		camera.quaternion = glm::normalize(camera.quaternion);
		//glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f)*camera.quaternion;

		std::vector<glm::mat4> shadowMatrices;
		if(settings->shadowEnabled) {
			for(int i=0 ; i < shadowFrameBuffers.size() ; ++i) {
				std::pair<RenderBuffer, int> pair = shadowFrameBuffers[i];
				RenderBuffer buffer = pair.first;
				int orthoSize = pair.second;
				shadowMatrices.push_back(light.getVP(&camera, orthoSize, near, far));	
			}
		}

		glm::mat4 viewProjection = camera.getVP();

		mainScene->processSpace();
		mainScene->setVisibility(viewProjection, shadowMatrices, &camera, &light, settings);

	
		glPolygonMode(GL_FRONT, GL_FILL);
		glPatchParameteri(GL_PATCH_VERTICES, 3); // Define the number of control points per patch
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // Or GL_FRONT
		glFrontFace(GL_CCW); // Ensure this matches your vertex data
		glEnable(GL_DEPTH_TEST);
		glLineWidth(2.0);
		glPointSize(4.0);	


		UniformBlock uniformBlock;
        uniformBlock.uintData = glm::vec4(0);
		uniformBlock.floatData = glm::vec4( time, 0.0, 0.0 ,0.0);
		uniformBlock.world = worldModel;
		uniformBlock.lightDirection = glm::vec4(light.direction, 0.0f);
		uniformBlock.cameraPosition = glm::vec4(camera.position, 0.0f);
		uniformBlock.set(DEBUG_FLAG, settings->debugEnabled);
		uniformBlock.set(TESSELATION_FLAG, settings->tesselationEnabled);
		uniformBlock.set(SHADOW_FLAG, settings->shadowEnabled);
		uniformBlock.set(LIGHT_FLAG, settings->lightEnabled);
		uniformBlock.set(PARALLAX_FLAG, settings->parallaxEnabled);
		uniformBlock.set(DEPTH_FLAG, true);
		uniformBlock.set(OVERRIDE_FLAG, false);
		uniformBlock.set(OPACITY_FLAG, false);
		uniformBlock.set(TRIPLANAR_FLAG, false); 



		// ================
		// Shadow component
		// ================
		if(settings->shadowEnabled) {
			for(int i=0 ; i < shadowFrameBuffers.size() ; ++i) {
				std::pair<RenderBuffer, int> pair = shadowFrameBuffers[i];
				RenderBuffer buffer = pair.first;

				glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
				glViewport(0, 0, buffer.width, buffer.height);
				glClear(GL_DEPTH_BUFFER_BIT);
		
				glm::mat4 lightProjection = shadowMatrices[i];
				uniformBlock.matrixShadow[i] = Math::getCanonicalMVP(lightProjection);
				uniformBlock.viewProjection = lightProjection;

				glUseProgram(program3d);
				uniformBlock.set(OPACITY_FLAG, false);
				program3dData->uniform(&uniformBlock);

				mainScene->draw3dSolid(camera.position, settings, &mainScene->visibleShadowNodes[i]);
				
				if(settings->billboardEnabled) {
					glUseProgram(programBillboard);
					uniformBlock.set(OPACITY_FLAG, true);
					programBillboardData->uniform(&uniformBlock);
					mainScene->drawBillboards(camera.position, settings, &mainScene->visibleShadowNodes[i]);
				}
			}
			viewerBlock = uniformBlock;
		}

		uniformBlock.viewProjection = viewProjection;

		// =================
		// First Pass: Depth
		// =================
		glBindFramebuffer(GL_FRAMEBUFFER, depthFrameBuffer.frameBuffer);
		glViewport(0, 0, depthFrameBuffer.width, depthFrameBuffer.height);
		glClear(GL_DEPTH_BUFFER_BIT);


		glUseProgram(program3d);
		uniformBlock.set(OPACITY_FLAG, false);
		program3dData->uniform(&uniformBlock);
		mainScene->draw3dSolid(camera.position, settings, &mainScene->visibleSolidNodes);

		uniformBlock.set(TESSELATION_FLAG, false);
		uniformBlock.set(OPACITY_FLAG, settings->opacityEnabled);
		if(settings->billboardEnabled) {
			glUseProgram(programBillboard);
			programBillboardData->uniform(&uniformBlock);
			mainScene->drawBillboards(camera.position, settings, &mainScene->visibleSolidNodes);
		}
		// ==================
		// Second Pass: Solid
		//===================
		glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer.frameBuffer);
		glViewport(0, 0, renderBuffer.width, renderBuffer.height);
		glClearColor (0.1,0.1,0.1,1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		uniformBlock.set(DEPTH_FLAG, false);

		if(settings->wireFrameEnabled) {
			uniformBlock.set(LIGHT_FLAG, false); 
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		} 
		if(settings->billboardEnabled) {
			glUseProgram(programBillboard);
			programBillboardData->uniform(&uniformBlock);
			mainScene->drawBillboards(camera.position, settings, &mainScene->visibleSolidNodes);
		}
		uniformBlock.set(TESSELATION_FLAG, settings->tesselationEnabled);
		uniformBlock.set(OPACITY_FLAG, false);
		uniformBlock.set(TRIPLANAR_FLAG, true); 

		glUseProgram(program3d);
		program3dData->uniform(&uniformBlock);
		mainScene->draw3dSolid(camera.position, settings, &mainScene->visibleSolidNodes);
		if(settings->wireFrameEnabled) {
			glPolygonMode(GL_FRONT, GL_FILL);
		}
	
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

		glUseProgram(program3d);
		program3dData->uniform(&uniformBlock);
		mainScene->draw3dLiquid(camera.position, settings, &mainScene->visibleLiquidNodes);

		//glUseProgram(program3d);
		brushEditor->draw3dIfOpen(&uniformBlock);
		
		glPolygonMode(GL_FRONT, GL_FILL);

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
				if (ImGui::MenuItem("Uniform Viewer", "Ctrl+M")) {
					uniformBlockViewer->show();
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
			
			
		
			
			ImGui::Text("%d FPS", framesPerSecond);
			ImGui::Text("%d/%d solid instances", mainScene->solidInstancesVisible, mainScene->solidInstancesCount);
			ImGui::Text("%d/%d liquid instances", mainScene->liquidInstancesVisible, mainScene->liquidInstancesCount);
			ImGui::Text("%d/%d vegetation instances", mainScene->vegetationInstancesVisible, mainScene->vegetationInstancesCount);
			ImGui::End();

		}

		uniformBlockViewer->draw2dIfOpen();
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

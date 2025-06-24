#define STB_PERLIN_IMPLEMENTATION
#include "math/math.hpp"
#include "gl/gl.hpp"
#include "ui/ui.hpp"
#include "command/command.hpp"
#include "tools/tools.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

class MainApplication : public LithosApplication {
	std::vector<UniformBlockBrush> brushes;
	std::vector<UniformBlockBrush> billboardBrushes;
	std::vector<AtlasTexture> atlasTextures;
	std::vector<AtlasParams> atlasParams;
	std::vector<MixerParams> mixers;
	std::vector<AnimateParams> animations;
	std::vector<ImpostorParams> impostors;
	Scene * mainScene;
	Settings * settings = new Settings();
	glm::mat4 worldModel = glm::mat4(1.0f); // Identity matrix

	//glm::quat quaternion = glm::normalize(glm::quat(1.0f, 1.0f, 1.0f, 1.0f));
	glm::quat quaternion =  Math::eulerToQuat(0, 45, 0);
	// pitch yaw roll
	
	//Camera camera = Camera(glm::vec3(21649, 600, 5271), quaternion , 0.1f, 8192.0f);
	Camera camera = Camera(glm::vec3(0, 500, 0), quaternion , 1.0f, 8192.0f);
	DirectionalLight light;

	GLuint programSwap;
	GLuint program3d;
	GLuint programBillboard;
	GLuint programImpostor;
	GLuint programDeferred;
	GLuint programAtlas;
	GLuint programTexture;
	GLuint programCopy;
	GLuint programDebug;
	GLuint programDepth;
	GLuint programMixTexture;
	GLuint programWaterTexture;

	ProgramData * uniformBlockData;
	ProgramData * uniformBrushData;
	Brush3d * brush3d;
	DrawableInstanceGeometry<InstanceData> * brushSphere;
	DrawableInstanceGeometry<InstanceData> * brushBox;

	UniformBlock viewerBlock;
	UniformBlock uniformBlock;

	TextureImage noiseTexture;
	TextureImage gamepadTexture;
	TextureBlitter * textureBlitter1024;
	TextureBlitter * textureBlitter256;
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
	CameraEditor * cameraEditor;
	TextureViewer * textureViewer;
	ImpostorViewer * impostorViewer;
	GamepadEditor * gamepadEditor;

	TextureLayers atlasLayers;
	TextureLayers textureLayers;
	TextureLayers billboardLayers;
	TextureLayers impostorLayers;
	
	ImpostorDrawer * impostorDrawer;
	TextureMixer * textureMixer;
	AnimatedTexture * textureAnimator;
	AtlasDrawer * atlasDrawer;
	GamepadControllerStrategy * gamepadControllerStrategy;
	KeyboardControllerStrategy * keyboardControllerStrategy;

public:
	MainApplication() {

	}

	~MainApplication() {

	}

	virtual void setup() {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_CLAMP);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // Or GL_FRONT
		glFrontFace(GL_CCW); // Ensure this matches your vertex data
		glDepthMask(GL_TRUE);  // Allow writing to depth buffer
		glLineWidth(2.0);
		glPointSize(4.0);	

		std::cout << "sizeof(Vertex) = " << sizeof(Vertex) << std::endl; 
		std::cout << "sizeof(OctreeNode) = " << sizeof(OctreeNode) << std::endl; 
		std::cout << "sizeof(ChildBlock) = " << sizeof(ChildBlock) << std::endl; 
		std::cout << "sizeof(OctreeNodeSerialized) = " << sizeof(OctreeNodeSerialized) << std::endl; 

		BoundingCube box(glm::vec3(0), 1);
		BoundingCube c0 = box.getChild(0);
		BoundingCube c1 = box.getChild(1);
		if(c0.intersects(c1)) {
			std::cout << "Neighbors Intersect" << std::endl;
		}

		// Register all GDAL drivers
		GDALAllRegister();

		mainScene = new Scene(settings);
		//mainScene->load("data");
	
		uniformBlockData = new ProgramData();
		uniformBrushData = new ProgramData();
		brush3d = new Brush3d();

		gamepadControllerStrategy = new GamepadControllerStrategy(camera, *brush3d, *mainScene);
		keyboardControllerStrategy = new KeyboardControllerStrategy(camera, *brush3d, *this, *mainScene);

		std::vector<GlslInclude> includes;
		includes.push_back(GlslInclude("#include<functions.glsl>" , readFile("shaders/util/functions.glsl")));
		includes.push_back(GlslInclude("#include<perlin.glsl>" , readFile("shaders/util/perlin.glsl")));
		includes.push_back(GlslInclude("#include<functions_fragment.glsl>" , readFile("shaders/util/functions_fragment.glsl")));
		includes.push_back(GlslInclude("#include<structs.glsl>" , readFile("shaders/util/structs.glsl")));
		includes.push_back(GlslInclude("#include<parallax.glsl>" , readFile("shaders/util/parallax.glsl")));
		includes.push_back(GlslInclude("#include<depth.glsl>" , readFile("shaders/util/depth.glsl")));
		includes.push_back(GlslInclude("#include<uniforms.glsl>" , readFile("shaders/util/uniforms.glsl")));
		includes.push_back(GlslInclude("#include<shadow.glsl>" , readFile("shaders/util/shadow.glsl")));
		includes.push_back(GlslInclude("#include<tbn.glsl>" , readFile("shaders/util/tbn.glsl")));
		includes.push_back(GlslInclude("#include<triplanar.glsl>" , readFile("shaders/util/triplanar.glsl")));

		programAtlas = createShaderProgram({
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/texture/atlas_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/texture/atlas_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});

		programSwap = createShaderProgram({
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/texture/swap_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/texture/swap_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});

		programTexture = createShaderProgram({
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/texture/texture_array_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/texture/texture_array_fragment.glsl")),GL_FRAGMENT_SHADER)
		});

		programCopy = createShaderProgram({
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/texture/copy_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/texture/copy_geometry.glsl")),GL_GEOMETRY_SHADER), 
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/texture/copy_fragment.glsl")),GL_FRAGMENT_SHADER)
		});

		programDepth = createShaderProgram({
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/texture/depth_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/texture/depth_fragment.glsl")),GL_FRAGMENT_SHADER)
		});

		programMixTexture = createShaderProgram({
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/texture/mix_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/texture/mix_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});

		programWaterTexture = createShaderProgram({
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/texture/water_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/texture/water_fragment.glsl")),GL_FRAGMENT_SHADER)
		});

		program3d = createShaderProgram({
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/3d_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/3d_tessControl.glsl")),GL_TESS_CONTROL_SHADER), 
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/3d_tessEvaluation.glsl")),GL_TESS_EVALUATION_SHADER),
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/3d_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});

		programBillboard = createShaderProgram({
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/3d_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/3d_tessControl.glsl")),GL_TESS_CONTROL_SHADER), 
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/3d_tessEvaluation.glsl")),GL_TESS_EVALUATION_SHADER),
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/3d_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});

		programDebug = createShaderProgram({
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/debug_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/debug_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});

		programImpostor = createShaderProgram({
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/impostor_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/impostor_geometry.glsl")),GL_GEOMETRY_SHADER), 
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/impostor_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});

		programDeferred = createShaderProgram({
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/deferred_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/deferred_geometry.glsl")),GL_GEOMETRY_SHADER), 
			compileShader(GlslInclude::replaceIncludes(includes,readFile("shaders/deferred_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});

		textureLayers.textures[0] = createTextureArray(1024, 1024, 20, GL_RGB8);
		textureLayers.textures[1] = createTextureArray(1024, 1024, 20, GL_RGB8);
		textureLayers.textures[2] = createTextureArray(1024, 1024, 20, GL_R8);
		textureLayers.count = 0;

		billboardLayers.textures[0] = createTextureArray(1024, 1024, 4, GL_RGB8);
		billboardLayers.textures[1] = createTextureArray(1024, 1024, 4, GL_RGB8);
		billboardLayers.textures[2] = createTextureArray(1024, 1024, 4, GL_R8);
		billboardLayers.count = 0;

		atlasLayers.textures[0] = createTextureArray(1024, 1024, 4, GL_RGB8);
		atlasLayers.textures[1] = createTextureArray(1024, 1024, 4, GL_RGB8);
		atlasLayers.textures[2] = createTextureArray(1024, 1024, 4, GL_R8);
		atlasLayers.count = 0;

		impostorLayers.textures[0] = createTextureArray(256, 256, 4, GL_RGB8);
		impostorLayers.textures[1] = createTextureArray(256, 256, 4, GL_RGB8);
		impostorLayers.textures[2] = createTextureArray(256, 256, 4, GL_R8);
		impostorLayers.count = 0;

		textureBlitter1024 = new TextureBlitter(programCopy, 1024, 1024, {GL_RGB8, GL_R8});
		textureBlitter256 = new TextureBlitter(programCopy, 256, 256, {GL_RGB8, GL_R8});

		renderBuffer = createRenderFrameBuffer(getWidth(), getHeight(), true);
		solidBuffer = createRenderFrameBuffer(getWidth(), getHeight(), true);
		depthFrameBuffer = createDepthFrameBuffer(getWidth(), getHeight());
		shadowFrameBuffers.push_back(std::pair(createDepthFrameBuffer(1024, 1024), 256));
		shadowFrameBuffers.push_back(std::pair(createDepthFrameBuffer(1024, 1024), 2048));
		shadowFrameBuffers.push_back(std::pair(createDepthFrameBuffer(1024, 1024), 8192));

		textureMixer = new TextureMixer(1024,1024, programMixTexture, &textureLayers, textureBlitter1024);
		textureAnimator = new AnimatedTexture(1024,1024, programWaterTexture ,&textureLayers, textureBlitter1024);
		atlasDrawer = new AtlasDrawer(programAtlas, 1024, 1024, &atlasLayers, &billboardLayers, textureBlitter1024);
		impostorDrawer = new ImpostorDrawer(programDeferred, 256, 256, &billboardLayers, &impostorLayers, textureBlitter256);

		{
			SphereGeometry sphereGeometry(40,80);
			BoxGeometry boxGeometry(BoundingBox(glm::vec3(-0.5), glm::vec3(0.5)));
			InstanceDataHandler handler;
			std::vector<InstanceData> instances;
			instances.push_back(InstanceData(0,glm::mat4(1.0),0));
			brushSphere = new DrawableInstanceGeometry<InstanceData>(&sphereGeometry, &instances, &handler);
			brushBox = new DrawableInstanceGeometry<InstanceData>(&boxGeometry, &instances, &handler);
		}

		{
			UniformBlockBrush tb = UniformBlockBrush(textureLayers.count, glm::vec2(0.2), 0.02, 8, 32, 16,4, 10.0, 0.5 , 1.33);
			animations.push_back(AnimateParams(textureLayers.count));
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			UniformBlockBrush tb = UniformBlockBrush(textureLayers.count, glm::vec2(0.2), 0.1, 8, 32, 16,4 ,256, 0.4, 0.0);
			loadTexture(&textureLayers, {"textures/lava_color.jpg", "textures/lava_normal.jpg", "textures/lava_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			UniformBlockBrush tb = UniformBlockBrush(textureLayers.count, glm::vec2(0.2), 0.01, 2, 8, 8,4 ,32, 0.03, 0.0);
			loadTexture(&textureLayers, {"textures/grass_color.jpg", "textures/grass_normal.jpg", "textures/grass_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
        }
		{
			UniformBlockBrush tb = UniformBlockBrush(textureLayers.count, glm::vec2(0.2), 0.05, 8, 32, 16,4 ,32,0.02, 0.0);
			loadTexture(&textureLayers, {"textures/sand_color.jpg", "textures/sand_normal.jpg", "textures/sand_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
        }
		{
			UniformBlockBrush tb = UniformBlockBrush(textureLayers.count, glm::vec2(0.2), 0.1, 8, 32, 16,4,128, 0.4, 0.0);
			loadTexture(&textureLayers, {"textures/rock_color.jpg", "textures/rock_normal.jpg", "textures/rock_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
        }
		{
			UniformBlockBrush tb = UniformBlockBrush(textureLayers.count, glm::vec2(0.2), 0.1, 8, 32, 16,4, 32 , 0.4, 0.0);
			loadTexture(&textureLayers, {"textures/snow_color.jpg", "textures/snow_normal.jpg", "textures/snow_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
        }
		{
			UniformBlockBrush tb = UniformBlockBrush(textureLayers.count, glm::vec2(0.2), 0.1, 8, 64, 64,4, 32, 0.6 , 0.0);
			loadTexture(&textureLayers, {"textures/metal_color.jpg", "textures/metal_normal.jpg", "textures/metal_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
        }
		{
			UniformBlockBrush tb = UniformBlockBrush(textureLayers.count, glm::vec2(0.2), 0.1, 8, 32, 16,4 , 256, 0.02, 0.0);
			loadTexture(&textureLayers, {"textures/dirt_color.jpg", "textures/dirt_normal.jpg", "textures/dirt_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
        }
		{
			UniformBlockBrush tb = UniformBlockBrush(textureLayers.count, glm::vec2(0.2), 0.01, 8, 32, 16,4, 256, 0.2 , 0.0);
			loadTexture(&textureLayers, {"textures/bricks_color.jpg", "textures/bricks_normal.jpg", "textures/bricks_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			UniformBlockBrush tb = UniformBlockBrush(textureLayers.count, glm::vec2(0.2), 0.01, 8, 32, 16,4, 256, 0.2 , 0.0);
			mixers.push_back(MixerParams(textureLayers.count, 2, 3));
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			UniformBlockBrush tb = UniformBlockBrush(textureLayers.count, glm::vec2(0.2), 0.01, 8, 32, 16,4, 256, 0.2 , 0.0);
			mixers.push_back(MixerParams(textureLayers.count, 2, 5));
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			UniformBlockBrush tb = UniformBlockBrush(textureLayers.count, glm::vec2(0.2), 0.01, 8, 32, 16,4, 256, 0.2 , 0.0);
			mixers.push_back(MixerParams(textureLayers.count, 4, 2));
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			UniformBlockBrush tb = UniformBlockBrush(textureLayers.count, glm::vec2(0.2), 0.01, 8, 32, 16,4, 256, 0.2 , 0.0);
			mixers.push_back(MixerParams(textureLayers.count, 4, 5));
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			UniformBlockBrush tb = UniformBlockBrush(textureLayers.count, glm::vec2(0.2), 0.01, 8, 32, 16,4, 256, 0.2 , 0.0);
			mixers.push_back(MixerParams(textureLayers.count, 4, 3));
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			UniformBlockBrush tb = UniformBlockBrush(textureLayers.count, glm::vec2(0.2), 0.01, 4, 16, 8,4, 256, 0.2 , 0.0);
			loadTexture(&textureLayers, {"textures/soft_sand_color.jpg", "textures/soft_sand_normal.jpg", "textures/soft_sand_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			UniformBlockBrush tb = UniformBlockBrush(textureLayers.count, glm::vec2(0.2), 0.01, 4, 16, 8,4, 256, 0.2 , 0.0);
			loadTexture(&textureLayers, {"textures/forest_color.jpg", "textures/forest_normal.jpg", "textures/forest_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			loadTexture(&atlasLayers, {"textures/vegetation/foliage_color.jpg", "textures/vegetation/foliage_normal.jpg", "textures/vegetation/foliage_opacity.jpg"}, billboardLayers.count, false);
			AtlasTexture at = AtlasTexture();
			at.tiles.push_back(Tile(glm::vec2(1.0),glm::vec2(0.0)));
			at.tiles.push_back(Tile(glm::vec2(0.15, 1.0),glm::vec2(0.0, 0.0)));
			at.tiles.push_back(Tile(glm::vec2(0.15, 0.5),glm::vec2(0.15, 0.0)));
			at.tiles.push_back(Tile(glm::vec2(0.15, 0.5),glm::vec2(0.15, 0.5)));
			at.tiles.push_back(Tile(glm::vec2(0.15, 0.5),glm::vec2(0.30, 0.0)));
			at.tiles.push_back(Tile(glm::vec2(0.15, 0.5),glm::vec2(0.30, 0.5)));
			at.tiles.push_back(Tile(glm::vec2(0.15, 0.5),glm::vec2(0.45, 0.0)));
			at.tiles.push_back(Tile(glm::vec2(0.15, 0.5),glm::vec2(0.45, 0.5)));
			at.tiles.push_back(Tile(glm::vec2(0.4, 0.5),glm::vec2(0.6, 0.0)));
			at.tiles.push_back(Tile(glm::vec2(0.4, 0.5),glm::vec2(0.6, 0.5)));

			//brushes.push_back(new Brush(at, glm::vec2(0.2), 0.02, 8, 32, 16,4, 10.0, 0.5 , 1.33));

			AtlasParams ap(atlasTextures.size() ,atlasParams.size());
			ap.draws.push_back(TileDraw(0,glm::vec2(1), glm::vec2(0.5), glm::vec2(0.5), 0.5));
			atlasParams.push_back(ap);

			atlasTextures.push_back(at);
			UniformBlockBrush tb = UniformBlockBrush(billboardLayers.count, glm::vec2(1.0));

			billboardBrushes.push_back(tb);
			++atlasLayers.count;
			++billboardLayers.count;
		}
		{
			loadTexture(&atlasLayers, {"textures/vegetation/grass_color.jpg", "textures/vegetation/grass_normal.jpg", "textures/vegetation/grass_opacity.jpg"}, billboardLayers.count, false);
			AtlasTexture at = AtlasTexture();
			at.tiles.push_back(Tile(glm::vec2(1.0),glm::vec2(0.0)));
			
			AtlasParams ap(atlasTextures.size() ,atlasParams.size());
			ap.draws.push_back(TileDraw(0,glm::vec2(1), glm::vec2(0.5), glm::vec2(0.5), 0.0));
			atlasParams.push_back(ap);

			atlasTextures.push_back(at);
			UniformBlockBrush tb = UniformBlockBrush(billboardLayers.count, glm::vec2(1.0));
			
			billboardBrushes.push_back(tb);
			++atlasLayers.count;
			++billboardLayers.count;
		}
		{
			InstanceDataHandler instanceHandler;
			std::vector<InstanceData> vegetationInstances;
			vegetationInstances.push_back(InstanceData(0, glm::mat4(1.0), 0));
			DrawableInstanceGeometry<InstanceData> * vegetationMesh = new DrawableInstanceGeometry<InstanceData>(new Vegetation3d(1), &vegetationInstances, &instanceHandler);
			impostors.push_back(ImpostorParams(impostorLayers.count++, vegetationMesh));
		}
		
		noiseTexture = loadTextureImage("textures/noise.jpg", false);
		gamepadTexture = loadTextureImage("textures/gamepad.png", true);

		Texture::bindTexture(program3d, activeTexture, glGetUniformLocation(program3d, "noise"), noiseTexture);
		activeTexture = Texture::bindTexture(programBillboard, activeTexture, glGetUniformLocation(programBillboard, "noise"), noiseTexture);

		Texture::bindTexture(program3d, activeTexture,  glGetUniformLocation(program3d, "depthTexture"), depthFrameBuffer.depthTexture);
		activeTexture = Texture::bindTexture(programBillboard, activeTexture,  glGetUniformLocation(programBillboard, "depthTexture"), depthFrameBuffer.depthTexture);

		Texture::bindTexture(program3d, activeTexture, glGetUniformLocation(program3d, "underTexture"), solidBuffer.colorTexture);
		activeTexture = Texture::bindTexture(programBillboard, activeTexture, glGetUniformLocation(programBillboard, "underTexture"), solidBuffer.colorTexture);

		for(size_t i=0; i < shadowFrameBuffers.size(); ++i) {
			std::string shadowMapName = "shadowMap["+ std::to_string(i) +"]";
			RenderBuffer *buffer = &shadowFrameBuffers[i].first;
			Texture::bindTexture(program3d, activeTexture, glGetUniformLocation(program3d, shadowMapName.c_str()), buffer->depthTexture);
			activeTexture = Texture::bindTexture(programBillboard, activeTexture, glGetUniformLocation(programBillboard, shadowMapName.c_str()), buffer->depthTexture);
		}
		
		for(int i =0; i < 3 ; ++i) {
			std::string objectName = "textures[" + std::to_string(i) + "]";
			Texture::bindTexture(programImpostor, activeTexture, objectName, billboardLayers.textures[i]);
			Texture::bindTexture(programDeferred, activeTexture, objectName, billboardLayers.textures[i]);
			activeTexture = Texture::bindTexture(programBillboard, activeTexture, objectName, billboardLayers.textures[i]);
			activeTexture = Texture::bindTexture(program3d, activeTexture, objectName, textureLayers.textures[i]);
		}

		std::cout << "Finished binding textures, activeTexture = " << std::to_string(activeTexture) << std::endl;

		glUseProgram(program3d);
		UniformBlockBrush::uniform(program3d, &brushes, "brushes", "brushTextures");

		glUseProgram(programBillboard);
		UniformBlockBrush::uniform(programBillboard, &billboardBrushes, "brushes", "brushTextures");

		glUseProgram(programImpostor);
		UniformBlockBrush::uniform(programImpostor, &billboardBrushes, "brushes", "brushTextures");

		glUseProgram(programDeferred);
		UniformBlockBrush::uniform(programDeferred, &billboardBrushes, "brushes", "brushTextures");

		for(MixerParams &params : mixers) {
			textureMixer->mix(params);
		}

		for(AtlasParams &params : atlasParams) {
			atlasDrawer->draw(params, &atlasTextures);
		}

		for(ImpostorParams &params : impostors) {
			impostorDrawer->draw(params, 0);
		}

        light.direction = glm::normalize(glm::vec3(-1.0,-1.0,-1.0));
		glUseProgram(0);
		//tesselator->normalize();
		uniformBlockViewer = new UniformBlockViewer(&viewerBlock);
		atlasPainter = new AtlasPainter(&atlasParams, &atlasTextures, atlasDrawer, programAtlas, programTexture, 256,256, &billboardLayers);
		atlasViewer = new AtlasViewer(&atlasTextures, atlasDrawer, programAtlas, programTexture, 256,256, &atlasLayers, programCopy);
		brushEditor = new BrushEditor(brush3d, &camera, &brushes, program3d, programTexture, &textureLayers);
		cameraEditor = new CameraEditor(&camera);
		gamepadEditor = new GamepadEditor(gamepadTexture);
		shadowMapViewer = new ShadowMapViewer(&shadowFrameBuffers, 512, 512);
		textureMixerEditor = new TextureMixerEditor(textureMixer, &mixers, programTexture, &textureLayers);
		animatedTextureEditor = new AnimatedTextureEditor(&animations, programTexture, 256,256, &textureLayers);
		depthBufferViewer = new DepthBufferViewer(programDepth,depthFrameBuffer.depthTexture,512,512, &camera);
		settingsEditor = new SettingsEditor(settings);
		textureViewer = new TextureViewer(programTexture, &textureLayers);
		impostorViewer = new ImpostorViewer(impostorDrawer, &impostors , programTexture, 256, 256, &impostorLayers);

		// ImGui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		(void)io;
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(getWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 460");


		//mainScene->generate(camera);
	}



	virtual void update(float deltaTime){
		time += deltaTime;
		gamepadControllerStrategy->handleInput(deltaTime);
		keyboardControllerStrategy->handleInput(deltaTime);
	//    camera.projection[1][1] *= -1;
	 //   modelMatrix = glm::rotate(modelMatrix, deltaTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		for(AnimateParams &params : animations) {
			textureAnimator->animate(time, params);
		}
    }

	// Camera settings
	glm::vec3 noiseScale = glm::vec3(256.0f, 64.0, 256.0);  // Controls movement magnitude
	glm::vec3 basePosition = glm::vec3(0.0f,32.0f, 0.0f); // Starting position

	// Camera settings
	float speed = 0.2f;
	float lerpSpeed = 0.1f;  // Controls how smoothly we transition between directions
	glm::vec3 lastDirection = glm::vec3(0.0f, 0.0f, 1.0f); // Initial direction, pointing forward

	// Function to compute Perlin noise-based camera position at a given time
	glm::vec3 getCameraPosition(float time) {
		float x = stb_perlin_noise3(time * speed, 0.0f, 0.0f, 0, 0, 0) * noiseScale[0];
		float y = stb_perlin_noise3(0.0f, time * speed, 100.0f, 0, 0, 0) * noiseScale[1];
		float z = stb_perlin_noise3(0.0f, 0.0f, time * speed, 0, 0, 0) * noiseScale[2];

		return basePosition + glm::vec3(x, y, z);
	}

	// Function to get the direction, smoothing it with Lerp
	glm::vec3 getDirection(float time) {
		glm::vec3 currentPos = getCameraPosition(time);
		glm::vec3 nextPos = getCameraPosition(time + 0.05f); // A small step forward in time

		// Compute the direction vector
		glm::vec3 direction = glm::normalize(nextPos - currentPos);

		// Linearly interpolate between the last direction and the current direction using glm::mix
		glm::vec3 smoothDirection = glm::mix(lastDirection, direction, lerpSpeed);

		// Update lastDirection for the next frame
		lastDirection = smoothDirection;

		return smoothDirection;
	}

	void drawBrush3d(ProgramData data){
		glUseProgram(program3d);
		UniformBlockBrush * brush = &brushes[brush3d->index];
		UniformBlockBrush::uniform(program3d, brush, "brushes", "brushTextures", brush3d->index, brush->textureIndex);	

		glm::mat4 model = glm::scale(
			glm::translate(  
				glm::mat4(1.0f), 
				brush3d->position
			), 
			brush3d->scale
		);

		uniformBlock.world = model;
		uniformBlock.set(OVERRIDE_FLAG, true);
		uniformBlock.uintData.w = uint(brush3d->index);

		UniformBlockBrush::uniform(program3d, brush, "overrideProps");
		UniformBlock::uniform(0, &uniformBlock, sizeof(UniformBlock) , &data);
		long count = 0;
		if(brush3d->shape == BrushShape::SPHERE) {
			brushSphere->draw(GL_PATCHES, &count);
		}else {
			brushBox->draw(GL_PATCHES, &count);
		}
	}

	float processTime = 0;


    virtual void draw3d() {
		// Convert quaternion to rotation matrix
		glm::mat4 rotate = glm::mat4_cast(glm::normalize(camera.quaternion)); 

		// Apply translation (negative because we move the world, not the camera)
		glm::mat4 translate = glm::translate(glm::mat4(1.0f), -camera.position);

		// Correct multiplication order: Rotation first, then translation
		camera.view = rotate * translate;

		// Perspective projection
		camera.projection = glm::perspective(glm::radians(45.0f), getWidth() / (float)getHeight(), camera.near, camera.far);

		//camera.position = getCameraPosition(time);
		//glm::vec3 future = camera.position + getDirection(time);

		//camera.view = glm::lookAt(camera.position, future, glm::vec3(0.0,1.0,0.0));
		//glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f)*camera.quaternion;
		
		std::vector<std::pair<glm::mat4, glm::vec3>> shadowMatrices;

		if(settings->shadowEnabled) {
			for(std::pair<RenderBuffer, int> pair : shadowFrameBuffers) {
				int orthoSize = pair.second;
				glm::vec3 lightPosition;
				glm::mat4 lightMatrix = light.getViewProjection(camera.position, orthoSize, camera.near, camera.far,lightPosition);
				shadowMatrices.push_back({lightMatrix, lightPosition});	
			}
		}

		glm::mat4 viewProjection = camera.getViewProjection();

		mainScene->setVisibility(viewProjection, shadowMatrices, camera);

        double startTime = glfwGetTime(); // Get elapsed time in seconds
		if(mainScene->processSpace()) {
	        double endTime = glfwGetTime(); // Get elapsed time in seconds
			processTime += endTime - startTime;
		}
		else {
			//this->close();
		}
		glPolygonMode(GL_FRONT, GL_FILL);
		glPatchParameteri(GL_PATCH_VERTICES, 3); // Define the number of control points per patch
		glEnable(GL_DEPTH_TEST);
		//glDepthFunc(GL_LESS);        // Default: Pass if fragment depth is less than stored depth

        uniformBlock.uintData = glm::uvec4(0u, 0u, settings->debugMode, settings->overrideBrush);
		uniformBlock.floatData[0] = glm::vec4( time, settings->blendSharpness, settings->parallaxDistance ,settings->parallaxPower);
		uniformBlock.floatData[1] = glm::vec4( camera.near , camera.far, 0, 0);
		uniformBlock.world = worldModel;
		uniformBlock.lightDirection = glm::vec4(light.direction, 0.0f);
		uniformBlock.cameraPosition = glm::vec4(camera.position, 0.0f);

		uniformBlock.set(DEBUG_FLAG, settings->debugEnabled);
		uniformBlock.set(TESSELATION_FLAG, settings->tesselationEnabled);
		uniformBlock.set(SHADOW_FLAG, settings->shadowEnabled);
		uniformBlock.set(LIGHT_FLAG, settings->lightEnabled);
		uniformBlock.set(PARALLAX_FLAG, settings->parallaxEnabled);
		uniformBlock.set(DEPTH_FLAG, true);
		uniformBlock.set(OVERRIDE_FLAG, settings->overrideEnabled);
		uniformBlock.set(OPACITY_FLAG, false);
		uniformBlock.set(BILLBOARD_FLAG, false); 
		viewerBlock = uniformBlock;

		// ================
		// Shadow component
		// ================
		if(settings->shadowEnabled) {
			int i =0;
			for(std::pair<glm::mat4, glm::vec3> pair : shadowMatrices){

				std::pair<RenderBuffer, int> pair2 = shadowFrameBuffers[i];
				RenderBuffer buffer = pair2.first;
				glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
				glViewport(0, 0, buffer.width, buffer.height);
				glClear(GL_DEPTH_BUFFER_BIT);
			
				glm::mat4 lightProjection = pair.first;
				uniformBlock.matrixShadow[i] = Math::getCanonicalMVP(lightProjection);
				uniformBlock.viewProjection = lightProjection;
				uniformBlock.cameraPosition = glm::vec4(camera.position, 0.0f);

				glUseProgram(program3d);
				uniformBlock.set(OPACITY_FLAG, false);
				uniformBlock.set(BILLBOARD_FLAG, false); 

				UniformBlock::uniform(0, &uniformBlock, sizeof(UniformBlock), uniformBlockData);
				if(settings->solidEnabled) {
					mainScene->draw3dSolid(camera.position, mainScene->visibleShadowNodes[i]);
				}
				
				if(settings->billboardEnabled) {
					glUseProgram(programBillboard);
					uniformBlock.set(OPACITY_FLAG, settings->opacityEnabled);
					uniformBlock.set(BILLBOARD_FLAG, settings->billboardEnabled); 
					UniformBlock::uniform(0, &uniformBlock, sizeof(UniformBlock), uniformBrushData);
					mainScene->drawVegetation(camera.position, mainScene->visibleShadowNodes[i]);
				}
				++i;
			}
		}

		uniformBlock.viewProjection = viewProjection;
		uniformBlock.cameraPosition = glm::vec4(camera.position, 0.0f);

		// =================
		// First Pass: Depth
		// =================
		glBindFramebuffer(GL_FRAMEBUFFER, depthFrameBuffer.frameBuffer);
		glViewport(0, 0, depthFrameBuffer.width, depthFrameBuffer.height);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Disable color writing
		glDepthMask(GL_TRUE);  // Enable depth writing
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(settings->solidEnabled) {
			glUseProgram(program3d);
			uniformBlock.set(BILLBOARD_FLAG, false); 
			uniformBlock.set(OPACITY_FLAG, false);
			UniformBlock::uniform(0, &uniformBlock, sizeof(UniformBlock), uniformBlockData);
			mainScene->draw3dSolid(camera.position, mainScene->visibleSolidNodes);
		}

		if(settings->billboardEnabled) {
			glUseProgram(programBillboard);
			uniformBlock.set(TESSELATION_FLAG, false);
			uniformBlock.set(BILLBOARD_FLAG, settings->billboardEnabled); 
			uniformBlock.set(OPACITY_FLAG, settings->opacityEnabled);
			UniformBlock::uniform(0, &uniformBlock, sizeof(UniformBlock), uniformBrushData);
			mainScene->drawVegetation(camera.position, mainScene->visibleSolidNodes);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// ==================
		// Second Pass: Solid
		//===================
		glBindFramebuffer(GL_FRAMEBUFFER, renderBuffer.frameBuffer);
		glViewport(0, 0, renderBuffer.width, renderBuffer.height);
		glClearColor (0.1,0.1,0.1,1.0);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);  // Re-enable color writing
		//glDepthMask(GL_FALSE); // Optional: Prevent depth overwriting
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		uniformBlock.set(DEPTH_FLAG, false);

		if(settings->wireFrameEnabled) {
			uniformBlock.set(LIGHT_FLAG, false); 
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		} 

		if(settings->billboardEnabled) {
			glUseProgram(programBillboard);
			uniformBlock.set(BILLBOARD_FLAG, settings->billboardEnabled); 
			uniformBlock.set(OPACITY_FLAG, settings->opacityEnabled);
			UniformBlock::uniform(0, &uniformBlock, sizeof(UniformBlock), uniformBrushData);
			mainScene->drawVegetation(camera.position, mainScene->visibleSolidNodes);
		}

		if(settings->solidEnabled) {
			glUseProgram(program3d);
			uniformBlock.set(BILLBOARD_FLAG, false); 
			uniformBlock.set(TESSELATION_FLAG, settings->tesselationEnabled);
			uniformBlock.set(OPACITY_FLAG, false);
			UniformBlock::uniform(0, &uniformBlock, sizeof(UniformBlock), uniformBlockData);
			mainScene->draw3dSolid(camera.position, mainScene->visibleSolidNodes);
		}

		if(settings->octreeWireframe) {
			glUseProgram(programDebug);
			UniformBlock::uniform(0, &uniformBlock, sizeof(UniformBlock), uniformBlockData);
			mainScene->draw3dOctree(camera.position, mainScene->visibleSolidNodes);
		}

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

		if(settings->wireFrameEnabled) {
			uniformBlock.set(LIGHT_FLAG, false); 
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		} 

		if(settings->liquidEnabled) {
			UniformBlock::uniform(0, &uniformBlock, sizeof(UniformBlock), uniformBlockData);
			mainScene->draw3dLiquid(camera.position, mainScene->visibleLiquidNodes);
		}

		if(brush3d->enabled) {
			drawBrush3d(*uniformBlockData);
		}	

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

	virtual void draw2d(float time) {
		// ...
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		//glm::vec2 myVec(0.0f);
		//glm::vec2 myDelta(0.0f);
	
		//MouseDragViewer::render(myVec, myDelta);
	
		glm::vec3 frontDirection = glm::vec3(0.0f, 0.0f, 1.0f)*camera.quaternion;
		glm::vec3 leftDirection = glm::vec3(1.0f, 0.0f, 0.0f)*camera.quaternion;

		frontDirection.y = 0;
		frontDirection = glm::normalize(frontDirection);

		leftDirection.y = 0;
		leftDirection = glm::normalize(leftDirection);

		//camera.position += (frontDirection * myDelta.y + leftDirection*myDelta.x)*0.01f;
	
		if (ImGui::BeginMainMenuBar()) {
			// File Menu
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Generate")) {
					mainScene->generate(camera);
				}

				std::string defaultFilename = "default.env";

				if (ImGui::MenuItem("Open", "Ctrl+O")) {
					IGFD::FileDialogConfig config;
					config.path = "."; // Start in the current directory
					config.flags = ImGuiFileDialogFlags_Modal; // Optional: Make it modal
					config.fileName = defaultFilename;
					ImGuiFileDialog::Instance()->OpenDialog("ChooseFolderDlgOpenKey", "Select a File", ".env", config);
				}

				if (ImGui::MenuItem("Save", "Ctrl+S")) {
					IGFD::FileDialogConfig config;
					config.path = "."; // Start in the current directory
					config.flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite; // Optional: Make it modal
					config.fileName = defaultFilename;
					ImGuiFileDialog::Instance()->OpenDialog("ChooseFolderDlgSaveKey", "Select a File", ".env", config);

				}

				if (ImGui::MenuItem("Import File")) {
					IGFD::FileDialogConfig config;
					config.path = "."; // Start in the current directory
					config.flags = ImGuiFileDialogFlags_Modal; // Optional: Make it modal
					ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose an heightmap", ".tif,.jpg,.png,.hgt", config);
				}

				if (ImGui::MenuItem("Exit")) {
					this->close();
				}

				ImGui::EndMenu();
			}

			if (ImGuiFileDialog::Instance()->Display("ChooseFolderDlgSaveKey", ImGuiWindowFlags_NoCollapse, ImVec2(500, 300), ImVec2(1600, 900))) {
				if (ImGuiFileDialog::Instance()->IsOk()) {
					auto instance = ImGuiFileDialog::Instance();
					std::string filePath = instance->GetFilePathName();
					std::string folderPath = instance->GetCurrentPath();
					std::cout << "Selected file: " << filePath << std::endl;
					EnvironmentFile environment ("solid", "liquid", "brushes", &camera);
					environment.save(filePath);
					Seriallizer<UniformBlockBrush>::serialize(folderPath + "/" + environment.brushesFilename+".bin", brushes);
					mainScene->save(folderPath, camera);
				}
				ImGuiFileDialog::Instance()->Close();
			}

			if (ImGuiFileDialog::Instance()->Display("ChooseFolderDlgOpenKey", ImGuiWindowFlags_NoCollapse, ImVec2(500, 300), ImVec2(1600, 900))) {
				if (ImGuiFileDialog::Instance()->IsOk()) {
					auto instance = ImGuiFileDialog::Instance();
					std::string filePath = instance->GetFilePathName();
					std::string folderPath = instance->GetCurrentPath();
					std::cout << "Selected file: " << filePath << std::endl;
					EnvironmentFile environment (filePath, &camera);
					brushes.clear();
					Seriallizer<UniformBlockBrush>::deserialize(folderPath + "/" + environment.brushesFilename+".bin", brushes);
					mainScene->load(folderPath, camera);
				}
				ImGuiFileDialog::Instance()->Close();
			}

			if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, ImVec2(400, 300), ImVec2(1600, 900))) {
				if (ImGuiFileDialog::Instance()->IsOk()) {
					std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
					mainScene->import(filePath, camera);
					std::cout << "Chosen file: " << filePath << std::endl;
				}
				ImGuiFileDialog::Instance()->Close();
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
				if (ImGui::MenuItem("Camera Editor", "Ctrl+B")) {
					cameraEditor->show();
				}	
				if (ImGui::MenuItem("Depth Buffer Viewer", "Ctrl+B")) {
					depthBufferViewer->show();
				}
				if (ImGui::MenuItem("Gamepad Editor", "Ctrl+B")) {
					gamepadEditor->show();
				}
				if (ImGui::MenuItem("Impostor Viewer", "Ctrl+B")) {
					impostorViewer->show();
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
			ImGui::Text("%ld/%ld solid instances", mainScene->solidInstancesVisible, mainScene->solidInfo.size());
			ImGui::Text("%ld/%ld liquid instances", mainScene->liquidInstancesVisible, mainScene->liquidInfo.size());
			ImGui::Text("%ld/%ld vegetation instances", mainScene->vegetationInstancesVisible, mainScene->vegetationInfo.size());
			ImGui::Text("%ld solid triangles", mainScene->solidTrianglesCount);
			ImGui::Text("%ld liquid triangles", mainScene->liquidTrianglesCount);
			ImGui::Text("%ld solid datas", mainScene->solidSpace->allocator.getAllocatedBlocksCount());
			ImGui::Text("%ld liquid datas", mainScene->liquidSpace->allocator.getAllocatedBlocksCount());
			ImGui::Text("%f process time", processTime);

			size_t allocatedBlocks = mainScene->solidSpace->allocator.getAllocatedBlocksCount();
			size_t blockSize = mainScene->solidSpace->allocator.getBlockSize();

			size_t allocatedChildren = mainScene->solidSpace->allocator.childAllocator.getAllocatedBlocksCount();
			size_t childrenSize = mainScene->solidSpace->allocator.childAllocator.getBlockSize();

			ImGui::Text("%ld (%ld KB) allocatted nodes",  allocatedBlocks*blockSize, allocatedBlocks*blockSize* sizeof(OctreeNode)/1024);
			ImGui::Text("%ld (%ld KB) allocatted children",  allocatedChildren*childrenSize, allocatedChildren*childrenSize* sizeof(ChildBlock)/1024);

			if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1)) {
				ImGui::Text("Gamepad detected");
			} 

			#ifdef MEM_HEADER
			ImGui::Text("%ld KB", usedMemory/1024);
			#endif
			ImGui::End();
		}

		uniformBlockViewer->draw2dIfOpen(time);
		animatedTextureEditor->draw2dIfOpen(time);
		brushEditor->draw2dIfOpen(time);
		cameraEditor->draw2dIfOpen(time);
		gamepadEditor->draw2dIfOpen(time);
		shadowMapViewer->draw2dIfOpen(time);
		textureMixerEditor->draw2dIfOpen(time);
		depthBufferViewer->draw2dIfOpen(time);
		settingsEditor->draw2dIfOpen(time);
		atlasViewer->draw2dIfOpen(time);
		atlasPainter->draw2dIfOpen(time);
		textureViewer->draw2dIfOpen(time);
		impostorViewer->draw2dIfOpen(time);

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

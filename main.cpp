#define STB_PERLIN_IMPLEMENTATION
#include "math/math.hpp"
#include "gl/gl.hpp"
#include "ui/ui.hpp"
#include "tools/tools.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
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

// Function to create a quaternion from yaw, pitch, roll
glm::quat CreateQuaternion(float yaw, float pitch, float roll) {
    // Convert degrees to radians
    float yawRad   = glm::radians(yaw);
    float pitchRad = glm::radians(pitch);
    float rollRad  = glm::radians(roll);

    // Create individual axis quaternions
    glm::quat qYaw   = glm::angleAxis(yawRad, glm::vec3(0, 1, 0));  // Rotate around Y
    glm::quat qPitch = glm::angleAxis(pitchRad, glm::vec3(1, 0, 0)); // Rotate around X
    glm::quat qRoll  = glm::angleAxis(rollRad, glm::vec3(0, 0, 1));  // Rotate around Z

    // Apply in Yaw -> Pitch -> Roll order (multiplication applies right to left)
    return qYaw * qPitch * qRoll;
}


glm::quat EulerToQuat(float yaw, float pitch, float roll) {
    // Convert degrees to radians
    float yawRad = glm::radians(yaw);
    float pitchRad = glm::radians(pitch);
    float rollRad = glm::radians(roll);

    // Construct quaternion in correct order (Yaw -> Pitch -> Roll)
    glm::quat qYaw   = glm::angleAxis(yawRad, glm::vec3(0, 1, 0));  // Rotate around Y
    glm::quat qPitch = glm::angleAxis(pitchRad, glm::vec3(1, 0, 0)); // Rotate around X
    glm::quat qRoll  = glm::angleAxis(rollRad, glm::vec3(0, 0, 1));  // Rotate around Z

    return qYaw * qPitch * qRoll; // Yaw first, then Pitch, then Roll
}


class MainApplication : public LithosApplication {
	std::vector<UniformBlockBrush*> brushes;
	std::vector<UniformBlockBrush*> billboardBrushes;
	std::vector<AtlasTexture*> atlasTextures;
	std::vector<UniformBlockBrush> uniformBlockBrushes;
	std::map<UniformBlockBrush*, GLuint > textureMapper;

	std::vector<AtlasParams> atlasParams;

	std::vector<MixerParams> mixers;
	std::vector<AnimateParams> animations;
	std::vector<ImpostorParams> impostors;
	Scene * mainScene;
	Settings * settings = new Settings();
	glm::mat4 worldModel = glm::mat4(1.0f); // Identity matrix

	//glm::quat quaternion = glm::normalize(glm::quat(1.0f, 1.0f, 1.0f, 1.0f));
	glm::quat quaternion =  EulerToQuat(0, 45, 0);
	// pitch yaw roll
	
	Camera camera = Camera(glm::vec3(1024, 512, 1024), quaternion , 0.1f, 4096.0f);
	DirectionalLight light;

	GLuint programSwap;
	GLuint program3d;
	GLuint programBillboard;
	GLuint programImpostor;
	GLuint programDeferred;
	GLuint programAtlas;
	GLuint programTexture;
	GLuint programCopy;
	GLuint programDepth;
	GLuint programMixTexture;
	GLuint programWaterTexture;

	ProgramData * uniformBlockData;
	ProgramData * uniformBrushData;

	UniformBlock viewerBlock;
	UniformBlock uniformBlock;


	TextureImage noiseTexture;
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
	TextureViewer * textureViewer;
	ImpostorViewer * impostorViewer;

	TextureLayers atlasLayers;
	TextureLayers textureLayers;
	TextureLayers billboardLayers;
	TextureLayers impostorLayers;
	
	ImpostorDrawer * impostorDrawer;
	TextureMixer * textureMixer;
	AnimatedTexture * textureAnimator;
	AtlasDrawer * atlasDrawer;

public:
	MainApplication() {

	}

	~MainApplication() {

	}



    virtual void setup() {
		// Register all GDAL drivers
		GDALAllRegister();

		uniformBlockData = new ProgramData();
		uniformBrushData = new ProgramData();

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

		programCopy = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/texture/copy_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/copy_geometry.glsl")),GL_GEOMETRY_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/texture/copy_fragment.glsl")),GL_FRAGMENT_SHADER)
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

		programBillboard = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/3d_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/3d_tessControl.glsl")),GL_TESS_CONTROL_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/3d_tessEvaluation.glsl")),GL_TESS_EVALUATION_SHADER),
			compileShader(replaceIncludes(includes,readFile("shaders/3d_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});

		programImpostor = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/impostor_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/impostor_geometry.glsl")),GL_GEOMETRY_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/impostor_fragment.glsl")),GL_FRAGMENT_SHADER) 
		});

		programDeferred = createShaderProgram({
			compileShader(replaceIncludes(includes,readFile("shaders/deferred_vertex.glsl")),GL_VERTEX_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/deferred_geometry.glsl")),GL_GEOMETRY_SHADER), 
			compileShader(replaceIncludes(includes,readFile("shaders/deferred_fragment.glsl")),GL_FRAGMENT_SHADER) 
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
		shadowFrameBuffers.push_back(std::pair(createDepthFrameBuffer(1024, 1024), 128));
		shadowFrameBuffers.push_back(std::pair(createDepthFrameBuffer(1024, 1024), 1024));
		shadowFrameBuffers.push_back(std::pair(createDepthFrameBuffer(1024, 1024), 2048));

		textureMixer = new TextureMixer(1024,1024, programMixTexture, &textureLayers, textureBlitter1024);
		textureAnimator = new AnimatedTexture(1024,1024, programWaterTexture ,&textureLayers, textureBlitter1024);
		atlasDrawer = new AtlasDrawer(programAtlas, 1024, 1024, &atlasLayers, &billboardLayers, textureBlitter1024);
		impostorDrawer = new ImpostorDrawer(programDeferred, 256, 256, &billboardLayers, &impostorLayers, textureBlitter256);

		{
			UniformBlockBrush * tb = new UniformBlockBrush( glm::vec2(0.2), 0.02, 8, 32, 16,4, 10.0, 0.5 , 1.33);
			textureMapper.insert({tb, textureLayers.count});
			animations.push_back(AnimateParams(textureLayers.count));
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			UniformBlockBrush * tb = new UniformBlockBrush( glm::vec2(0.2), 0.1, 8, 32, 16,4 ,256, 0.4, 0.0);
			textureMapper.insert({tb, textureLayers.count});
			loadTexture(&textureLayers, {"textures/lava_color.jpg", "textures/lava_normal.jpg", "textures/lava_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			UniformBlockBrush * tb = new UniformBlockBrush( glm::vec2(0.2), 0.01, 2, 8, 8,4 ,32, 0.03, 0.0);
			textureMapper.insert({tb, textureLayers.count});
			loadTexture(&textureLayers, {"textures/grass_color.jpg", "textures/grass_normal.jpg", "textures/grass_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
        }
		{
			UniformBlockBrush * tb = new UniformBlockBrush( glm::vec2(0.2), 0.05, 8, 32, 16,4 ,32,0.02, 0.0);
			textureMapper.insert({tb, textureLayers.count});
			loadTexture(&textureLayers, {"textures/sand_color.jpg", "textures/sand_normal.jpg", "textures/sand_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
        }
		{
			UniformBlockBrush * tb = new UniformBlockBrush( glm::vec2(0.2), 0.1, 8, 32, 16,4,128, 0.4, 0.0);
			textureMapper.insert({tb, textureLayers.count});
			loadTexture(&textureLayers, {"textures/rock_color.jpg", "textures/rock_normal.jpg", "textures/rock_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
        }
		{
			UniformBlockBrush * tb = new UniformBlockBrush( glm::vec2(0.2), 0.1, 8, 32, 16,4, 32 , 0.4, 0.0);
			textureMapper.insert({tb, textureLayers.count});
			loadTexture(&textureLayers, {"textures/snow_color.jpg", "textures/snow_normal.jpg", "textures/snow_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
        }
		{
			UniformBlockBrush * tb = new UniformBlockBrush( glm::vec2(0.2), 0.1, 8, 64, 64,4, 32, 0.6 , 0.0);
			textureMapper.insert({tb, textureLayers.count});
			loadTexture(&textureLayers, {"textures/metal_color.jpg", "textures/metal_normal.jpg", "textures/metal_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
        }
		{
			UniformBlockBrush * tb = new UniformBlockBrush( glm::vec2(0.2), 0.1, 8, 32, 16,4 , 256, 0.02, 0.0);
			textureMapper.insert({tb, textureLayers.count});
			loadTexture(&textureLayers, {"textures/dirt_color.jpg", "textures/dirt_normal.jpg", "textures/dirt_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
        }
		{
			UniformBlockBrush * tb = new UniformBlockBrush( glm::vec2(0.2), 0.01, 8, 32, 16,4, 256, 0.2 , 0.0);
			textureMapper.insert({tb, textureLayers.count});
			loadTexture(&textureLayers, {"textures/bricks_color.jpg", "textures/bricks_normal.jpg", "textures/bricks_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			UniformBlockBrush * tb = new UniformBlockBrush( glm::vec2(0.2), 0.01, 8, 32, 16,4, 256, 0.2 , 0.0);
			textureMapper.insert({tb, textureLayers.count});
			mixers.push_back(MixerParams(textureLayers.count, 2, 3));
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			UniformBlockBrush * tb = new UniformBlockBrush( glm::vec2(0.2), 0.01, 8, 32, 16,4, 256, 0.2 , 0.0);
			textureMapper.insert({tb, textureLayers.count});
			mixers.push_back(MixerParams(textureLayers.count, 2, 5));
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			UniformBlockBrush * tb = new UniformBlockBrush( glm::vec2(0.2), 0.01, 8, 32, 16,4, 256, 0.2 , 0.0);
			textureMapper.insert({tb, textureLayers.count});
			mixers.push_back(MixerParams(textureLayers.count, 4, 2));
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			UniformBlockBrush * tb = new UniformBlockBrush( glm::vec2(0.2), 0.01, 8, 32, 16,4, 256, 0.2 , 0.0);
			textureMapper.insert({tb, textureLayers.count});
			mixers.push_back(MixerParams(textureLayers.count, 4, 5));
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			UniformBlockBrush * tb = new UniformBlockBrush( glm::vec2(0.2), 0.01, 8, 32, 16,4, 256, 0.2 , 0.0);
			textureMapper.insert({tb, textureLayers.count});
			mixers.push_back(MixerParams(textureLayers.count, 4, 3));
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			UniformBlockBrush * tb = new UniformBlockBrush( glm::vec2(0.2), 0.01, 4, 16, 8,4, 256, 0.2 , 0.0);
			textureMapper.insert({tb, textureLayers.count});
			loadTexture(&textureLayers, {"textures/soft_sand_color.jpg", "textures/soft_sand_normal.jpg", "textures/soft_sand_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			UniformBlockBrush * tb = new UniformBlockBrush( glm::vec2(0.2), 0.01, 4, 16, 8,4, 256, 0.2 , 0.0);
			textureMapper.insert({tb, textureLayers.count});
			loadTexture(&textureLayers, {"textures/forest_color.jpg", "textures/forest_normal.jpg", "textures/forest_bump.jpg"}, textureLayers.count, true);
			brushes.push_back(tb);
			textureLayers.count++;
		}
		{
			loadTexture(&atlasLayers, {"textures/vegetation/foliage_color.jpg", "textures/vegetation/foliage_normal.jpg", "textures/vegetation/foliage_opacity.jpg"}, billboardLayers.count, false);
			AtlasTexture * at = new AtlasTexture();
			
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

			//brushes.push_back(new Brush(at, glm::vec2(0.2), 0.02, 8, 32, 16,4, 10.0, 0.5 , 1.33));

			AtlasParams ap(atlasTextures.size() ,atlasParams.size(), at);
			ap.draws.push_back(TileDraw(0,glm::vec2(1), glm::vec2(0.5), glm::vec2(0.5), 0.5));
			atlasParams.push_back(ap);

			atlasTextures.push_back(at);
			UniformBlockBrush * tb = new UniformBlockBrush(glm::vec2(1.0));
			textureMapper.insert({tb, billboardLayers.count++});

			billboardBrushes.push_back(tb);
			++atlasLayers.count;
		}
		{
			loadTexture(&atlasLayers, {"textures/vegetation/grass_color.jpg", "textures/vegetation/grass_normal.jpg", "textures/vegetation/grass_opacity.jpg"}, billboardLayers.count, false);
			AtlasTexture * at = new AtlasTexture();
			at->tiles.push_back(Tile(glm::vec2(1.0),glm::vec2(0.0)));
			
			AtlasParams ap(atlasTextures.size() ,atlasParams.size(), at);
			ap.draws.push_back(TileDraw(0,glm::vec2(1), glm::vec2(0.5), glm::vec2(0.5), 0.0));
			atlasParams.push_back(ap);

			atlasTextures.push_back(at);
			UniformBlockBrush * tb = new UniformBlockBrush(glm::vec2(1.0));
			
			textureMapper.insert({tb, billboardLayers.count++});

			billboardBrushes.push_back(tb);
			++atlasLayers.count;
		}
		{
			std::vector<InstanceData> vegetationInstances;
			vegetationInstances.push_back(InstanceData(0, glm::mat4(1.0), 0));
			DrawableInstanceGeometry * vegetationMesh = new DrawableInstanceGeometry(TYPE_INSTANCE_VEGETATION_DRAWABLE, new Vegetation3d(1), &vegetationInstances);
			impostors.push_back(ImpostorParams(impostorLayers.count++, vegetationMesh));
		}
		
		noiseTexture = loadTextureImage("textures/noise.jpg", false);

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
		UniformBlockBrush::uniform(program3d,&brushes, "brushes", "brushTextures", &textureMapper);

		glUseProgram(programBillboard);
		UniformBlockBrush::uniform(programBillboard,&billboardBrushes, "brushes", "brushTextures", &textureMapper);

		glUseProgram(programImpostor);
		UniformBlockBrush::uniform(programImpostor,&billboardBrushes, "brushes", "brushTextures", &textureMapper);

		glUseProgram(programDeferred);
		UniformBlockBrush::uniform(programDeferred,&billboardBrushes, "brushes", "brushTextures", &textureMapper);


		for(MixerParams &params : mixers) {
			textureMixer->mix(params);
		}

		for(AtlasParams &params : atlasParams) {
			atlasDrawer->draw(params);
		}

		for(ImpostorParams &params : impostors) {
			impostorDrawer->draw(params, 0);
		}

		mainScene = new Scene(settings);
		mainScene->load();
	


        light.direction = glm::normalize(glm::vec3(-1.0,-1.0,-1.0));
		glUseProgram(0);
		//tesselator->normalize();
		uniformBlockViewer = new UniformBlockViewer(&viewerBlock);
		atlasPainter = new AtlasPainter(&atlasParams, &atlasTextures, atlasDrawer, programAtlas, programTexture, 256,256, &billboardLayers);
		atlasViewer = new AtlasViewer(&atlasTextures, atlasDrawer, programAtlas, programTexture, 256,256, &atlasLayers, programCopy);
		brushEditor = new BrushEditor(uniformBlockData,&camera, &brushes, program3d, programTexture, &textureLayers, &textureMapper);
		shadowMapViewer = new ShadowMapViewer(&shadowFrameBuffers, 512, 512);
		textureMixerEditor = new TextureMixerEditor(textureMixer, &mixers, programTexture, &textureLayers);
		animatedTextureEditor = new AnimatedTextureEditor(&animations, programTexture, 256,256, &textureLayers);
		depthBufferViewer = new DepthBufferViewer(programDepth,depthFrameBuffer.depthTexture,512,512);
		settingsEditor = new SettingsEditor(settings, &camera);
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

	}

    virtual void update(float deltaTime){
		time += deltaTime;
	   	if (getKeyboardStatus(GLFW_KEY_ESCAPE) != GLFW_RELEASE) {
		   	close();
		}

	//    camera.projection[1][1] *= -1;
	 //   modelMatrix = glm::rotate(modelMatrix, deltaTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	   	float rsense = 0.05;

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
		//glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f)*camera.quaternion;
		glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f)*camera.quaternion;

	   	float tsense = deltaTime*1000;
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

    virtual void draw3d() {
		float far = 4096.0f;
		float near = 0.1f;

		// Convert quaternion to rotation matrix
		glm::mat4 rotate = glm::mat4_cast(glm::normalize(camera.quaternion)); 

		// Apply translation (negative because we move the world, not the camera)
		glm::mat4 translate = glm::translate(glm::mat4(1.0f), -camera.position);

		// Correct multiplication order: Rotation first, then translation
		camera.view = rotate * translate;

		// Perspective projection
		camera.projection = glm::perspective(glm::radians(45.0f), getWidth() / (float)getHeight(), near, far);

		
		//camera.position = getCameraPosition(time);
		//glm::vec3 future = camera.position + getDirection(time);

		//camera.view = glm::lookAt(camera.position, future, glm::vec3(0.0,1.0,0.0));
		//glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f)*camera.quaternion;

		
		std::vector<std::pair<glm::mat4, glm::vec3>> shadowMatrices;

		if(settings->shadowEnabled) {
			for(std::pair<RenderBuffer, int> pair : shadowFrameBuffers) {
				int orthoSize = pair.second;
				glm::vec3 lightPosition;
				glm::mat4 lightMatrix = light.getViewProjection(camera.position, orthoSize, near, far,lightPosition);
				shadowMatrices.push_back({lightMatrix, lightPosition});	
			}
		}

		glm::mat4 viewProjection = camera.getViewProjection();

		mainScene->setVisibility(viewProjection, shadowMatrices, camera);
		mainScene->processSpace();

	
		glPolygonMode(GL_FRONT, GL_FILL);
		glPatchParameteri(GL_PATCH_VERTICES, 3); // Define the number of control points per patch
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // Or GL_FRONT
		glFrontFace(GL_CCW); // Ensure this matches your vertex data
		glEnable(GL_DEPTH_TEST);
		//glDepthFunc(GL_LESS);        // Default: Pass if fragment depth is less than stored depth
		glDepthMask(GL_TRUE);  // Allow writing to depth buffer
		glLineWidth(2.0);
		glPointSize(4.0);	


        uniformBlock.uintData = glm::uvec4(0u, 0u, settings->debugMode, settings->overrideBrush);
		uniformBlock.floatData = glm::vec4( time, settings->blendSharpness, settings->parallaxDistance ,settings->parallaxPower);
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

				mainScene->draw3dSolid(camera.position, mainScene->visibleShadowNodes[i]);
				
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


		glUseProgram(program3d);
		uniformBlock.set(BILLBOARD_FLAG, false); 
		uniformBlock.set(OPACITY_FLAG, false);

		UniformBlock::uniform(0, &uniformBlock, sizeof(UniformBlock), uniformBlockData);
		mainScene->draw3dSolid(camera.position, mainScene->visibleSolidNodes);


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



		glUseProgram(program3d);
		uniformBlock.set(BILLBOARD_FLAG, false); 
		uniformBlock.set(TESSELATION_FLAG, settings->tesselationEnabled);
		uniformBlock.set(OPACITY_FLAG, false);
		UniformBlock::uniform(0, &uniformBlock, sizeof(UniformBlock), uniformBlockData);
		mainScene->draw3dSolid(camera.position, mainScene->visibleSolidNodes);
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

		UniformBlock::uniform(0, &uniformBlock, sizeof(UniformBlock), uniformBlockData);
		mainScene->draw3dLiquid(camera.position, mainScene->visibleLiquidNodes);

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

	virtual void draw2d(float time) {
		// ...
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		

		glm::vec2 myVec(0.0f);
		glm::vec2 myDelta(0.0f);
	
		MouseDragViewer::render(myVec, myDelta);
	
		glm::vec3 frontDirection = glm::vec3(0.0f, 0.0f, 1.0f)*camera.quaternion;
		glm::vec3 leftDirection = glm::vec3(1.0f, 0.0f, 0.0f)*camera.quaternion;

		frontDirection.y = 0;
		frontDirection = glm::normalize(frontDirection);

		leftDirection.y = 0;
		leftDirection = glm::normalize(leftDirection);

		camera.position += (frontDirection * myDelta.y + leftDirection*myDelta.x)*0.01f;
	
		if (ImGui::BeginMainMenuBar()) {
			// File Menu
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New")) {
					mainScene->create();
				}
				if (ImGui::MenuItem("Open", "Ctrl+O")) {
					mainScene->load();
				}
				if (ImGui::MenuItem("Save", "Ctrl+S")) {
					mainScene->save();
					std::vector<UniformBlockBrush> vec;
					for(UniformBlockBrush * ubb : brushes){
						vec.push_back(*ubb);
					}
					UniformBlockBrush::save(&vec, "data/brushes.bin");
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
			ImGui::Text("%ld/%ld solid instances", mainScene->solidInstancesVisible, mainScene->solidInstancesCount);
			ImGui::Text("%ld/%ld liquid instances", mainScene->liquidInstancesVisible, mainScene->liquidInstancesCount);
			ImGui::Text("%ld/%ld vegetation instances", mainScene->vegetationInstancesVisible, mainScene->vegetationInstancesCount);
			#ifdef MEM_HEADER
			ImGui::Text("%ld KB", usedMemory/1024);
			#endif
			ImGui::End();

		}



		uniformBlockViewer->draw2dIfOpen(time);
		animatedTextureEditor->draw2dIfOpen(time);
		brushEditor->draw2dIfOpen(time);
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

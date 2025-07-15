#ifndef UI_HPP
#define UI_HPP

#include <vector>
#include "../gl/gl.hpp"
#include "../tools/tools.hpp"

class TexturePreviewer {
    RenderBuffer previewBuffer;
    RenderBuffer renderBuffer;
    GLuint previewProgram;
    GLuint previewVao;
    TextureLayers * layers;
    std::vector<std::string> layerNames;
    int selectedLayer;
    int width;
    int height;
public:
    TexturePreviewer(GLuint previewProgram, int width, int height, std::initializer_list<std::string> layerNames, TextureLayers * layers);
    void draw2d(int index);

};

class Closable {
    public:
    bool open = false;
    void show();
    void hide();
    bool isOpen();
    
    virtual void draw2d(float time) = 0;

    void draw2dIfOpen(float time){
        if(open) {
            draw2d(time);
        }
    }

};

class TextureViewer: public Closable {
    TexturePreviewer * previewer;
  	int selectedTexture = 0;
    TextureLayers * layers;
    public:
    TextureViewer(GLuint previewProgram, TextureLayers * layers);
    void draw2d(float time) override;
};

class UniformBlockViewer: public Closable {

    public:
    UniformBlock * block;
    UniformBlockViewer(UniformBlock * block);
    void draw2d(float time) override;
};


class AtlasViewer: public Closable {
    std::vector<AtlasTexture> * atlasTextures;
    TexturePreviewer * previewer;
    AtlasDrawer * drawer;
  	int selectedTexture = 0;
  	int selectedTile = 0;
    public:
    AtlasViewer(std::vector<AtlasTexture> * atlasTextures, AtlasDrawer * drawer, GLuint programAtlas, GLuint previewProgram, int width, int height, TextureLayers * sourceLayers, GLuint copyProgram) ;
    void draw2d(float time) override;
};

class AtlasPainter: public Closable {
    std::vector<AtlasParams> * atlasParams;
    std::vector<AtlasTexture> * atlasTextures;
    AtlasDrawer * atlasDrawer;
    TexturePreviewer * previewer;
  	int selectedDrawer = 0;
  	int selectedDraw = 0;

    public:
    AtlasPainter(std::vector<AtlasParams> * atlasParams, std::vector<AtlasTexture> * atlasTextures, AtlasDrawer * atlasDrawer, GLuint programAtlas, GLuint previewProgram, int width, int height, TextureLayers * layers);
    void draw2d(float time);
};

class ImpostorViewer: public Closable {
    ImpostorDrawer* impostorDrawer;
    TexturePreviewer * previewer;
    TextureLayers * layers;
    std::vector<ImpostorParams> * impostors;
    int selectedDrawer = 0;
    public:
    ImpostorViewer(ImpostorDrawer* impostorDrawer, std::vector<ImpostorParams> * impostors ,GLuint previewProgram, int width, int height, TextureLayers * layers);
    void draw2d(float time) override;
};


class BrushEditor: public Closable {
    std::vector<UniformBlockBrush> * brushes;
    GLuint program;
    Brush3d * brush;
    Scene * scene;
    Camera * camera;
    TexturePreviewer * previewer;
    BrushContext * brushContext;
    float yaw, pitch, roll;
    
    public:
    BrushEditor(Brush3d * brush, Camera * camera, std::vector<UniformBlockBrush> * brushes, GLuint program3d, GLuint previewProgram, TextureLayers * layers, Scene * scene, BrushContext * brushContext);
    void draw2d(float time) override;
    int getSelectedBrush();
    void resetPosition();
};

class CameraEditor: public Closable {
    Camera * camera;
  
    public:
    CameraEditor(Camera * camera);
    void draw2d(float time) override;
};

class GamepadEditor: public Closable {
    TextureImage gamepadTexture;
  
    public:
    GamepadEditor(TextureImage gamepadTexture);
    void draw2d(float time) override;
};


class ShadowMapViewer : public Closable{
	std::vector<std::pair<RenderBuffer, int>> * shadowBuffers;
  	int selectedBuffer;
    int width;
    int height;

    public:
    ShadowMapViewer(std::vector<std::pair<RenderBuffer, int>> * shadowBuffers, int width, int height);

    void draw2d(float time) override;
};


class DepthBufferViewer : public Closable{
    TextureImage depthTexture;
    RenderBuffer previewBuffer;
    GLuint program;
    GLuint previewVao;
    int width;
    int height;
    Camera * camera;
    public:
    DepthBufferViewer(GLuint previewProgram, TextureImage depthTexture, int width, int height, Camera * camera);

    void draw2d(float time) override;
};

class TextureMixerEditor : public Closable{
    TextureMixer * mixer;
    std::vector<MixerParams> * mixers;
    TexturePreviewer * previewer;
  	int selectedMixer;
    TextureLayers * layers;
    public:
    TextureMixerEditor(TextureMixer * mixer, std::vector<MixerParams> * mixers, GLuint previewProgram, TextureLayers * layers);
    void draw2d(float time) override;
};

class AnimatedTextureEditor : public Closable{
    std::vector<AnimateParams> * animations;
  	int selectedAnimatedTexture;
    TexturePreviewer * previewer;
    public:
    AnimatedTextureEditor(std::vector<AnimateParams> * animations, GLuint previewProgram, int width, int height, TextureLayers * layers);

    void draw2d(float time) override;
};

class ImageViewer : public Closable{
    GLuint texture;
    int width;
    int height;

    public:
    ImageViewer(GLuint texture, int width, int height);

    void draw2d(float time) override;
};

class SettingsEditor : public Closable {
    Settings * settings;
    public:
    SettingsEditor(Settings * settings);

    void draw2d(float time) override;
};

class MouseDragViewer {
    public:
    static void render(glm::vec2 &vec, glm::vec2 &delta);

};


#endif
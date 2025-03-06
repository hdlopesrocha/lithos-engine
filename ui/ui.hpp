#ifndef UI_HPP
#define UI_HPP

#include <vector>
#include "../gl/gl.hpp"

enum BrushMode {
    ADD, REMOVE, REPLACE, COUNT
};




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
    
    virtual void draw2d() = 0;
    virtual void draw3d(UniformBlock * block) = 0;

    void draw2dIfOpen(){
        if(open) {
            draw2d();
        }
    }

    void draw3dIfOpen(UniformBlock * block){
        if(open) {
            draw3d(block);
        } 
    }
};

class TextureViewer: public Closable {
    TexturePreviewer * previewer;
  	int selectedTexture = 0;
    TextureLayers * layers;
    public:
    TextureViewer(GLuint previewProgram, TextureLayers * layers);
    void draw2d();
    void draw3d(UniformBlock * block);
};

class UniformBlockViewer: public Closable {

    public:
    UniformBlock * block;
    UniformBlockViewer(UniformBlock * block);
    void draw2d();
    void draw3d(UniformBlock * block);
};


class AtlasViewer: public Closable {
    std::vector<AtlasTexture*> * atlasTextures;
    TexturePreviewer * previewer;
    AtlasDrawer * drawer;
  	int selectedTexture = 0;
  	int selectedTile = 0;
    public:
    AtlasViewer(std::vector<AtlasTexture*> * atlasTextures, AtlasDrawer * drawer, GLuint programAtlas, GLuint previewProgram, int width, int height, TextureLayers * sourceLayers ) ;
    void draw2d();
    void draw3d(UniformBlock * block);
};

class AtlasPainter: public Closable {
    std::vector<AtlasParams> * atlasParams;
    std::vector<AtlasTexture*> * atlasTextures;
    AtlasDrawer * atlasDrawer;
    TexturePreviewer * previewer;
  	int selectedDrawer = 0;
  	int selectedDraw = 0;

    public:
    AtlasPainter(std::vector<AtlasParams> * atlasParams, std::vector<AtlasTexture*> * atlasTextures, AtlasDrawer * atlasDrawer, GLuint programAtlas, GLuint previewProgram, int width, int height, TextureLayers * layers);
    void draw2d();
    void draw3d(UniformBlock * block);
};

class ImpostorViewer: public Closable {
    std::vector<ImpostorDrawer*> * impostorDrawers;
    TexturePreviewer * previewer;
    int selectedDrawer = 0;
    public:
    ImpostorViewer(std::vector<ImpostorDrawer*> * impostorDrawers, GLuint previewProgram, int width, int height, TextureLayers * layers);
    void draw2d();
    void draw3d(UniformBlock * block);
};


class BrushEditor: public Closable {
    std::vector<Brush*> * brushes;
    GLuint program;
    ProgramData * data;
	DrawableGeometry * sphere;
    Camera * camera;
  
    TexturePreviewer * previewer;
    BrushMode mode;
  	int selectedBrush = 0;
    glm::vec3 brushPosition;
    float brushRadius;

    public:
    BrushEditor(Camera * camera,std::vector<Brush*> * brushes, GLuint program3d, GLuint previewProgram, TextureLayers * layers);
    void draw2d();
    void draw3d(UniformBlock * block);
    int getSelectedBrush();
    void resetPosition();
};

class ShadowMapViewer : public Closable{
	std::vector<std::pair<RenderBuffer, int>> * shadowBuffers;
  	int selectedBuffer;
    int width;
    int height;

    public:
    ShadowMapViewer(std::vector<std::pair<RenderBuffer, int>> * shadowBuffers, int width, int height);

    void draw2d();
    void draw3d(UniformBlock * block);
};


class DepthBufferViewer : public Closable{
    TextureImage depthTexture;
    RenderBuffer previewBuffer;
    GLuint previewProgram;
    GLuint previewVao;
    int width;
    int height;
    public:
    DepthBufferViewer(GLuint previewProgram, TextureImage depthTexture, int width, int height);

    void draw2d();
    void draw3d(UniformBlock * block);
};

class TextureMixerEditor : public Closable{
    TextureMixer * mixer;
    std::vector<MixerParams> * mixers;
    TexturePreviewer * previewer;
  	int selectedMixer;
    TextureLayers * layers;
    public:
    TextureMixerEditor(TextureMixer * mixer, std::vector<MixerParams> * mixers, GLuint previewProgram, TextureLayers * layers);
    void draw2d();
    void draw3d(UniformBlock * block);
};

class AnimatedTextureEditor : public Closable{
    std::vector<AnimateParams> * animations;
  	int selectedAnimatedTexture;
    TexturePreviewer * previewer;
    public:
    AnimatedTextureEditor(std::vector<AnimateParams> * animations, GLuint previewProgram, int width, int height, TextureLayers * layers);

    void draw2d();
    void draw3d(UniformBlock * block);
};

class ImageViewer : public Closable{
    GLuint texture;
    int width;
    int height;

    public:
    ImageViewer(GLuint texture, int width, int height);

    void draw2d();
    void draw3d(UniformBlock * block);
};

class SettingsEditor : public Closable {
    Settings * settings;

    public:
    SettingsEditor(Settings * settings);

    void draw2d();
    void draw3d(UniformBlock * block);
};

#endif
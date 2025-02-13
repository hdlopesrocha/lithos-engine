#ifndef UI_HPP
#define UI_HPP

#include <vector>
#include "../gl/gl.hpp"

enum BrushMode {
    ADD, REMOVE, REPLACE, COUNT
};



class TexturePreviewer {
    RenderBuffer previewBuffer;
    GLuint previewProgram;
    GLuint previewVao;
    std::vector<std::string> layers;
    int selectedLayer;
    int width;
    int height;
public:
    TexturePreviewer(GLuint previewProgram, int width, int height, std::initializer_list<std::string> layers);
    void draw2d(TextureArray texture);

};

class Closable {
    public:
    bool open = false;
    void show();
    void hide();
    bool isOpen();
    
    virtual void draw2d() = 0;
    virtual void draw3d() = 0;

    void draw2dIfOpen(){
        if(open) {
            draw2d();
        }
    }

    void draw3dIfOpen(){
        if(open) {
            draw3d();
        } 
    }
};

class TextureViewer: public Closable {
    std::vector<Texture*> * textures;
    TexturePreviewer * previewer;
  	int selectedTexture = 0;

    public:
    TextureViewer(std::vector<Texture*> * textures, GLuint previewProgram);
    void draw2d();
    void draw3d();
};

class AtlasViewer: public Closable {
    std::vector<AtlasTexture*> * textures;
    std::vector<std::string> layers;
    RenderBuffer previewBuffer;
    GLuint previewProgram;
    GLuint previewVao;
  	int selectedTexture = 0;
  	int selectedLayer = 0;
  	int selectedTile = 0;
    int width;
    int height;

    public:
    AtlasViewer(std::vector<AtlasTexture*> * textures, GLuint previewProgram, int width, int height);
    void draw2d();
    void draw3d();
};

class BrushEditor: public Closable {
    std::vector<Brush*> * brushes;
    GLuint program3d;
	DrawableGeometry * sphere;
    Camera * camera;
	GLuint modelLoc;
	GLuint modelViewProjectionLoc;
	GLuint shadowEnabledLoc;
	GLuint overrideTextureEnabledLoc;
	GLuint overrideTextureLoc;
  
    TexturePreviewer * previewer;
    BrushMode mode;
  	int selectedBrush = 0;
    Brush * brush;
    glm::vec3 brushPosition;
    float brushRadius;

    public:
    BrushEditor(Camera * camera,std::vector<Brush*> * brushes, GLuint program3d, GLuint previewProgram);
    void draw2d();
    void draw3d();
    int getSelectedBrush();
    void resetPosition();
};

class ShadowMapViewer : public Closable{
    GLuint shadowTexture;

    public:
    ShadowMapViewer(GLuint shadowTexture);

    void draw2d();
    void draw3d();
};


class DepthBufferViewer : public Closable{
    GLuint depthTexture;
    RenderBuffer previewBuffer;
    GLuint previewProgram;
    GLuint previewVao;
    int width;
    int height;
    public:
    DepthBufferViewer(GLuint previewProgram, GLuint depthTexture, int width, int height);

    void draw2d();
    void draw3d();
};

class TextureMixerEditor : public Closable{
    std::vector<TextureMixer*> * mixers;
    TexturePreviewer * previewer;
    std::vector<Texture*> * textures;
  	int selectedMixer;
    
    public:
    TextureMixerEditor(std::vector<TextureMixer*> * mixers, std::vector<Texture*> * textures, GLuint previewProgram);
    void draw2d();
    void draw3d();
};

class AnimatedTextureEditor : public Closable{
    std::vector<AnimatedTexture*> * animatedTextures;
    std::vector<Texture*> * textures;
  	int selectedAnimatedTexture;
    TexturePreviewer * previewer;
    public:
    AnimatedTextureEditor(std::vector<AnimatedTexture*> * animatedTextures, std::vector<Texture*> * textures, GLuint previewProgram, int width, int height);

    void draw2d();
    void draw3d();
};

class ImageViewer : public Closable{
    GLuint texture;
    int width;
    int height;

    public:
    ImageViewer(GLuint texture, int width, int height);

    void draw2d();
    void draw3d();
};

class SettingsEditor : public Closable {
    Settings * settings;

    public:
    SettingsEditor(Settings * settings);

    void draw2d();
    void draw3d();
};

#endif
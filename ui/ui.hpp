#include <vector>
#include "../gl/gl.hpp"

enum BrushMode {
    ADD, REMOVE, REPLACE, COUNT
};

class TexturePreviewer {
    RenderBuffer previewBuffer;
    GLuint previewProgram;
    GLuint previewVao;
    int selectedLayer;
    int width;
    int height;
public:
    TexturePreviewer(GLuint previewProgram, int width, int height);
    void draw2d(TextureArray texture);

};

class BrushEditor {
    bool open = false;
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
    void show();
    void hide();
    bool isOpen();
    void draw2d();
    void draw3d();
    int getSelectedBrush();
    void resetPosition();
};

class ShadowMapViewer {
    bool open = false;
    GLuint shadowTexture;

    public:
    ShadowMapViewer(GLuint shadowTexture);
    void show();
    void hide();
    bool isOpen();
    void draw2d();
    void draw3d();
};

class TextureMixerEditor {
    bool open = false;
    std::vector<TextureMixer*> * mixers;
    TexturePreviewer * previewer;
    std::vector<Texture*> * textures;
  	int selectedMixer;
    
    public:
    TextureMixerEditor(std::vector<TextureMixer*> * mixers, std::vector<Texture*> * textures, GLuint previewProgram);
    void show();
    void hide();
    bool isOpen();
    void draw2d();
    void draw3d();
};

class AnimatedTextureEditor {
    bool open = false;
    std::vector<AnimatedTexture*> * animatedTextures;
    std::vector<Texture*> * textures;
  	int selectedAnimatedTexture;
    TexturePreviewer * previewer;

    public:
    AnimatedTextureEditor(std::vector<AnimatedTexture*> * animatedTextures, std::vector<Texture*> * textures, GLuint previewProgram);
    void show();
    void hide();
    bool isOpen();
    void draw2d();
    void draw3d();
};


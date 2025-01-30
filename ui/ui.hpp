#include <vector>
#include "../gl/gl.hpp"

enum BrushMode {
    ADD, REMOVE, REPLACE, COUNT
};


class BrushEditor {
    bool open = false;
    std::vector<Brush*> * brushes;
    GLuint previewProgram;
    GLuint program3d;
    RenderBuffer previewBuffer;
    GLuint previewVao;
	DrawableGeometry * sphere;
    Camera * camera;
	GLuint modelLoc;
	GLuint modelViewProjectionLoc;
	GLuint shadowEnabledLoc;
	GLuint overrideTextureEnabledLoc;
	GLuint overrideTextureLoc;
  
    int selectedLayer;
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
    RenderBuffer previewBuffer;
    std::vector<Texture*> * textures;
    GLuint previewProgram;
    GLuint previewVao;
  	int selectedMixer;
    int selectedLayer;
    
    public:
    TextureMixerEditor(std::vector<TextureMixer*> * mixers, std::vector<Texture*> * textures, GLuint previewProgram);
    void show();
    void hide();
    bool isOpen();
    void draw2d();
    void draw3d();
};


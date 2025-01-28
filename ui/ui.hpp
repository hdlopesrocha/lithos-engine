#include <vector>
#include "../gl/gl.hpp"

enum BrushMode {
    ADD, REMOVE, REPLACE, COUNT
};


class BrushEditor {
    bool open = false;
    std::vector<Texture*> * textures;
	int selectedTexture = 0;
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
    BrushMode mode;
    Texture texture;
    glm::vec3 brushPosition;
    float brushRadius;

    public:
    BrushEditor(Camera * camera,std::vector<Texture*> * t, GLuint program3d, GLuint previewProgram, RenderBuffer previewBuffer, GLuint previewVao);
    void show();
    void hide();
    bool isOpen();
    void draw2d();
    void draw3d();
    int getSelectedTexture();
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
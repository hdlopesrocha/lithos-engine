#include <vector>
#include "../gl/gl.hpp"



class BrushEditor {
    bool open = false;
    std::vector<Texture*> * textures;
	int selectedTexture = 0;
    GLuint previewProgram;
    RenderBuffer previewBuffer;
    GLuint previewVao;

    public:
    BrushEditor(std::vector<Texture*> * t, GLuint previewProgram, RenderBuffer previewBuffer, GLuint previewVao);
    void show();
    void hide();
    bool isOpen();
    void render();
    int getSelectedTexture();
};

class ShadowMapViewer {
    bool open = false;
    GLuint shadowTexture;

    public:
    ShadowMapViewer(GLuint shadowTexture);
    void show();
    void hide();
    bool isOpen();
    void render();
};
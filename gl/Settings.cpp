
#include "gl.hpp"

Settings::Settings() {
    this->parallaxEnabled = true;
    this->shadowEnabled = true;
    this->lightEnabled = true;
    this->debugEnabled = false;
    this->wireFrameEnabled = false;
    this->tesselationEnabled = true;
    this->opacityEnabled = true;
    this->billboardEnabled = true;
    this->billboardRange = 128;
    this->overrideTexture = 0;
    this->debugMode = 0;
}

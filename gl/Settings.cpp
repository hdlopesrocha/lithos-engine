
#include "gl.hpp"

Settings::Settings() {
    this->parallaxEnabled = true;
    this->shadowEnabled = true;
    this->lightEnabled = true;
    this->debugEnabled = false;
    this->wireFrameEnabled = false;
    this->tesselationEnabled = true;
    this->opacityEnabled = true;
    this->billboardEnabled = false;
    this->billboardRange = 128;
}

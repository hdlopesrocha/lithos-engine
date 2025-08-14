
#include "gl.hpp"

Settings::Settings() {
    this->solidEnabled = true;
    this->liquidEnabled = true;
    this->parallaxEnabled = true;
    this->shadowEnabled = true;
    this->lightEnabled = true;
    this->debugEnabled = false;
    this->wireFrameEnabled = false;
    this->tesselationEnabled = true;
    this->opacityEnabled = true;
    this->billboardEnabled = true;
    this->billboardRange = 512;
    this->overrideBrush = 0;
    this->debugMode = 0;
    this->overrideEnabled = false;
    this->blendSharpness = 10.0;
    this->parallaxDistance = 1024.0;
    this->parallaxPower = 100.0;
    this->octreeWireframe = false;
    this->safetyDetailRatio = 0.05f;
}

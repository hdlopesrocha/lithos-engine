#include "gl.hpp"


TileDraw::TileDraw(uint index, glm::vec2 size, glm::vec2 offset, float angle){
    this->index = index;
    this->size = size;
    this->offset = offset;
    this->angle = angle;
}

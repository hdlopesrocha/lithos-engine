#include "math.hpp"


TileDraw::TileDraw(uint index, glm::vec2 size, glm::vec2 offset, glm::vec2 pivot, float angle){
    this->index = index;
    this->size = size;
    this->offset = offset;
    this->pivot = pivot;
    this->angle = angle;
}

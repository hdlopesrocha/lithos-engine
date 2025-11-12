#include "tools.hpp"

 
SimpleBrush::SimpleBrush(int brush){
    this->brush = brush;
}

int SimpleBrush::paint(const Vertex &vertex, glm::vec4 translate, glm::vec4 scale) const {
    return brush;
}

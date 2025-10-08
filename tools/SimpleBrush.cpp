#include "tools.hpp"

 
SimpleBrush::SimpleBrush(int brush){
    this->brush = brush;
}

int SimpleBrush::paint(const Vertex &vertex) const {
    return brush;
}

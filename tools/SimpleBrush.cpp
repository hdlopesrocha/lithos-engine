#include "tools.hpp"

 
SimpleBrush::SimpleBrush(int brush){
    this->brush = brush;
}

void SimpleBrush::paint(Vertex &vertex) const {
    vertex.brushIndex = brush;
}

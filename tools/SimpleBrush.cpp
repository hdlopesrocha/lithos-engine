#include "tools.hpp"

 
SimpleBrush::SimpleBrush(Brush * brush){
    this->brush = brush;
}

void SimpleBrush::paint(Vertex * vertex) const {
    vertex->brushIndex = brush->brushIndex;
}

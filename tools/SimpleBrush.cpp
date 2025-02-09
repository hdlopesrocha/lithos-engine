#include "tools.hpp"

 
SimpleBrush::SimpleBrush(Texture * texture){
    this->texture = texture;
}

void SimpleBrush::paint(Vertex * vertex) {
    vertex->texIndex = texture->index;
}

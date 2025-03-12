#include "gl.hpp"

TextureProperties::TextureProperties(uint textureIndex, UniformBlockBrush brush){
    this->brush = brush;
    this->textureIndex = textureIndex;
}




std::vector<UniformBlockBrush> convert(std::vector<TextureProperties> &vec) {
    std::vector<UniformBlockBrush> data;

    for(TextureProperties &tp : vec) {
        data.push_back(tp.brush);
    }

    return data;
}



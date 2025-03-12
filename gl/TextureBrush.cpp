#include "gl.hpp"

TextureBrush::TextureBrush(uint textureIndex, UniformBlockBrush brush){
    this->brush = brush;
    this->textureIndex = textureIndex;
}




std::vector<UniformBlockBrush> convert(std::vector<TextureBrush> &vec) {
    std::vector<UniformBlockBrush> data;

    for(TextureBrush &tp : vec) {
        data.push_back(tp.brush);
    }

    return data;
}



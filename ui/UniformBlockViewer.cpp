#include "ui.hpp"


UniformBlockViewer::UniformBlockViewer(UniformBlock * block) {
    this->block = block;
}

void UniformBlockViewer::draw2d(){
    ImGui::Begin("Uniform Block Viewer", &open, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextUnformatted(UniformBlock::toString(block).c_str());

    ImGui::End();
}

void UniformBlockViewer::draw3d(UniformBlock block){

}
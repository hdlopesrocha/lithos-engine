#include "ui.hpp"


OctreeExplorer::OctreeExplorer(Octree * tree) : tree(tree) {
}   


void OctreeExplorer::recursiveDraw(OctreeNode * node) {
    std::string blockId ="blockId = " + std::to_string(node->id);
    ImGui::Text(blockId.c_str());

    std::string flags ="";
    flags += node->isDirty() ? "*":"_"; 
    flags += node->isChunk() ? "X":"_";
    flags += node->isSolid() ? "S":"_";
    flags += node->isEmpty() ? "E":"_";
    flags += node->isLeaf() ? "/":"_";
    flags += node->isSimplified() ? "#":"_"; 
    std::string bitsText ="bits = " + flags ;
    ImGui::Text(bitsText.c_str());

    std::string sdf ="[";
    for(int i=0; i < 8; ++i) {
        if(i>0) {
            sdf += ", ";
        }
        float s = node->sdf[i];
        sdf += s == INFINITY ? "inf" : std::to_string(s);
    }
    sdf += "]";
    std::string sdfText ="sdf = " + sdf ;
    ImGui::Text(sdfText.c_str());

    std::string brushText ="brush = " + std::to_string(node->vertex.brushIndex);
    ImGui::Text(brushText.c_str());


    ChildBlock * block = node->getBlock(*tree->allocator);
    if(block != NULL) {
        for(int i =0 ; i < 8 ; ++i) {
            OctreeNode * child = block->get(i, *tree->allocator);
            if(child != NULL) {
                std::string nodeName = "children[" + std::to_string(i) + "] = " + std::to_string(block->children[i]);     
                if (ImGui::TreeNode(nodeName.c_str())) {
                    
                        recursiveDraw(child);
                    ImGui::TreePop();
                }
            }
        }
    }
    
}


void OctreeExplorer::draw2d(float time){

    ImGui::Begin("Octree Explorer", &open, ImGuiWindowFlags_AlwaysAutoResize);
  
    recursiveDraw(tree->root);

	ImGui::End();
}

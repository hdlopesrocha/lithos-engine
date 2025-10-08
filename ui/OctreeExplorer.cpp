#include "ui.hpp"


OctreeExplorer::OctreeExplorer(Octree * tree) : tree(tree) {
}   

void OctreeExplorer::recursiveDraw(OctreeNode * node, BoundingCube cube) {
    std::string blockId ="blockId = " + std::to_string(node->id);
    ImGui::Text(blockId.c_str());
    openNodes.push_back({cube, node});

    {
        glm::vec3 rgb = Math::brushColor(node->vertex.brushIndex);
        ImVec4 color(node->isSolid() ? 1.0 : 0.5, node->isEmpty() ? 1.0 : 0.5, node->isChunk() ? 1.0 : 0.5, 1.0f);
        std::string flags ="";
        flags += node->isDirty() ? "*":"_"; 
        flags += node->isChunk() ? "X":"_";
        flags += node->isSolid() ? "S":"_";
        flags += node->isEmpty() ? "E":"_";
        flags += node->isLeaf() ? "/":"_";
        flags += node->isSimplified() ? "#":"_"; 
        std::string bitsText ="bits = " + flags ;
        ImGui::PushStyleColor(ImGuiCol_Text, color); // Red text
        ImGui::Text(bitsText.c_str());
        ImGui::PopStyleColor();
    }
    {
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
    }
    {
        glm::vec3 rgb = Math::brushColor(node->vertex.brushIndex);
        ImVec4 color(rgb.r, rgb.g, rgb.b, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, color); // Red text
        std::string brushText ="brush = " + std::to_string(node->vertex.brushIndex);
        ImGui::Text(brushText.c_str());
        ImGui::PopStyleColor();
    }

    ChildBlock * block = node->getBlock(*tree->allocator);
    if(block != NULL) {
        for(int i =0 ; i < 8 ; ++i) {
            OctreeNode * child = block->get(i, *tree->allocator);
            if(child != NULL) {
                std::string nodeName = "children[" + std::to_string(i) + "] = " + std::to_string(block->children[i]);     
                if (ImGui::TreeNode(nodeName.c_str())) {
                    recursiveDraw(child, cube.getChild(i));
                    ImGui::TreePop();
                }
            }
        }
    }
    
}


void OctreeExplorer::draw2d(float time){

    ImGui::Begin("Octree Explorer", &open, ImGuiWindowFlags_AlwaysAutoResize);
    openNodes.clear();
    recursiveDraw(tree->root, *tree);

	ImGui::End();
}

#include "ui.hpp"


OctreeExplorer::OctreeExplorer(Octree * tree) : tree(tree) {
}   

void OctreeExplorer::recursiveDraw(OctreeNode * node, BoundingCube cube, uint level) {
    std::string blockId ="blockId = " + std::to_string(node->id);
    ImGui::Text(blockId.c_str());
    openNodes.push_back({cube, node});

    {
        SpaceType type = node->getType();
        glm::vec3 rgb = Math::brushColor(node->vertex.brushIndex);
        ImVec4 color(type == SpaceType::Solid ? 1.0 : 0.5, type == SpaceType::Empty ? 1.0 : 0.5, node->isChunk() ? 1.0 : 0.5, 1.0f);
        std::string flags ="";
        flags += node->isDirty() ? "*":"_"; 
        flags += node->isChunk() ? "C":"_";
        flags += type == SpaceType::Solid ? "S": type == SpaceType::Empty ? "E" : "-";
        flags += node->isLeaf() ? "/":"_";
        flags += node->isSimplified() ? "#":"_"; 
        std::string text ="bits = " + flags ;
        ImGui::PushStyleColor(ImGuiCol_Text, color); // Red text
        ImGui::Text(text.c_str());
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
        std::string text ="sdf = " + sdf ;
        ImGui::Text(text.c_str());
    }
    {
        glm::vec3 rgb = Math::brushColor(node->vertex.brushIndex);
        ImVec4 color(rgb.r, rgb.g, rgb.b, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, color); // Red text
        std::string text ="brush = " + std::to_string(node->vertex.brushIndex);
        ImGui::Text(text.c_str());
        ImGui::PopStyleColor();
    }
    {
        std::string text ="level = " + std::to_string(level);
        ImGui::Text(text.c_str());
    }
    ChildBlock * block = node->getBlock(*tree->allocator);
    if(block != NULL) {
        for(int i =0 ; i < 8 ; ++i) {
            OctreeNode * child = block->get(i, *tree->allocator);
            if(child != NULL) {
                std::string nodeName = "children[" + std::to_string(i) + "] = " + std::to_string(block->children[i]);     
                if (ImGui::TreeNode(nodeName.c_str())) {
                    recursiveDraw(child, cube.getChild(i), level +1);
                    ImGui::TreePop();
                }
            }
        }
    }
    
}


void OctreeExplorer::draw2d(float time){

    ImGui::Begin("Octree Explorer", &open, ImGuiWindowFlags_AlwaysAutoResize);
    openNodes.clear();
    recursiveDraw(tree->root, *tree, 0);

	ImGui::End();
}

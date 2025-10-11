#include "ui.hpp"


OctreeExplorer::OctreeExplorer(Octree * tree) : tree(tree) {
}   

// Convert HSV → RGB
glm::vec3 hsv2rgb(const glm::vec3& c)
{
    const glm::vec4 K = glm::vec4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
    glm::vec3 p = glm::abs(glm::fract(glm::vec3(c.x) + glm::vec3(K.x, K.y, K.z)) * 6.0f - glm::vec3(K.w));
    return c.z * glm::mix(glm::vec3(K.x), glm::clamp(p - glm::vec3(K.x), 0.0f, 1.0f), c.y);
}

// Generate brush color
glm::vec3 brushColor(unsigned int i)
{
    float hue = glm::fract(float(i) * 0.61803398875f); // Golden ratio spread
    return hsv2rgb(glm::vec3(hue, 0.7f, 0.9f));
}

void OctreeExplorer::recursiveDraw(OctreeNode * node) {
    std::string blockId ="blockId = " + std::to_string(node->id);
    ImGui::Text(blockId.c_str());

    {
        glm::vec3 rgb = brushColor(node->vertex.brushIndex);
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
        glm::vec3 rgb = brushColor(node->vertex.brushIndex);
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

#include "ui.hpp"

// Convert from glm::vec2 to ImVec2
ImVec2 ToImVec2(const glm::vec2& vec) {
    return ImVec2(vec.x, vec.y);
}

// Convert from ImVec2 to glm::vec2
glm::vec2 ToGlmVec2(const ImVec2& vec) {
    return glm::vec2(vec.x, vec.y);
}




static bool isDragging = false;
static glm::vec2 dragStartPos(0.0f, 0.0f);  // Starting position of the drag
static glm::vec2 currentPos(0.0f, 0.0f);   // Current position of the vector being dragged


void MouseDragViewer::render(glm::vec2 &vec, glm::vec2 &delta) {
    // Get the ImDrawList from the current window
    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    // Get the mouse position
    ImVec2 mousePos = ImGui::GetMousePos();
    vec = glm::vec2(mousePos.x, mousePos.y);

    // Check for mouse button press
    if (ImGui::IsMouseClicked(0)) {
        // Start dragging, save the initial position of the vector and mouse
        isDragging = true;
        dragStartPos = ToGlmVec2(mousePos);  // Convert mouse position to glm::vec2
        currentPos = vec;  // Store the current position of the vector
    }

    // If dragging, update the position of the vector
    if (isDragging) {
        // Get the difference between the current mouse position and the start position
        delta = ToGlmVec2(mousePos) - dragStartPos;

        // Update the vector based on the mouse movement (delta)
        vec = currentPos + delta;


        drawList->AddLine(ToImVec2(dragStartPos), mousePos, IM_COL32(255, 0, 0, 255), 2.0f);  // Red line
    }

    // Check if mouse button is released to stop dragging
    if (ImGui::IsMouseReleased(0)) {
        isDragging = false;
    }




}
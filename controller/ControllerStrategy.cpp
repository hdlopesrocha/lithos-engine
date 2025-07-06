#include "controller.hpp"

glm::vec3 ControllerStrategy::applyDeadzone(glm::vec3 input, float threshold) {
    glm::vec3 result = input;
    for (int i = 0; i < 3; ++i) {
        if (std::abs(result[i]) < threshold) {
            result[i] = 0.0f;
        }
    }
    return result;
}

bool ControllerStrategy::isAboveDeadzone(const glm::vec3& v, float threshold) {
    return glm::any(glm::greaterThan(glm::abs(v), glm::vec3(threshold)));
}
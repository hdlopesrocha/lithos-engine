#include "command.hpp"

glm::vec3 ControllerStrategy::applyDeadzone(glm::vec3 input, float threshold) {
    glm::vec3 absInput = glm::abs(input);
    glm::vec3 mask = glm::step(glm::vec3(threshold), absInput); // 0 if < threshold, 1 otherwise
    return input * mask;
}

bool ControllerStrategy::isAboveDeadzone(const glm::vec3& v, float threshold) {
    return glm::any(glm::greaterThan(glm::abs(v), glm::vec3(threshold)));
}
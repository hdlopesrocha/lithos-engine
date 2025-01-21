#include "math.hpp"

bool Math::isBetween(float x, float min, float max) {
	return min <= x && x <= max;
}

int Math::clamp(int val, int min, int max) {
	return val < min ? min : val > max ? max : val;
}

float Math::clamp(float val, float min, float max) {
	return val < min ? min : val > max ? max : val;
}


int Math::triplanarPlane(glm::vec3 position, glm::vec3 normal) {
    glm::vec3 absNormal = glm::abs(normal);
    if (absNormal.x > absNormal.y && absNormal.x > absNormal.z) {
        return normal.x > 0 ? 0 : 1;
    } else if (absNormal.y > absNormal.x && absNormal.y > absNormal.z) {
        return normal.y > 0 ? 2 : 3;
    } else {
        return normal.z > 0 ? 4 : 5;
    }
}

glm::vec2 Math::triplanarMapping(glm::vec3 position, int plane) {
    switch (plane) {
        case 0: return glm::vec2(-position.z, -position.y);
        case 1: return glm::vec2(position.z, -position.y);
        case 2: return glm::vec2(position.x, position.z);
        case 3: return glm::vec2(position.x, -position.z);
        case 4: return glm::vec2(position.x, -position.y);
        case 5: return glm::vec2(-position.x, -position.y);
        default: return glm::vec2(0.0,0.0);
    }
}

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

int Math::mod(int a, int b) {
    return (a % b + b) % b;
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


glm::vec3 Math::surfaceNormal(glm::vec3 point, BoundingBox box) {


    glm::vec3 d = (point - box.getCenter())/box.getLength(); // Vector from center to the point
    glm::vec3 ad = glm::abs(d); // Absolute values of components

    glm::vec3 v = glm::vec3(0);
    // Determine the dominant axis
    if (ad.x >= ad.y && ad.x >= ad.z) {
        v+= glm::vec3((d.x > 0? 1.0f : -1.0f), 0.0f, 0.0f); // Normal along X
    } 
    
    if (ad.y >= ad.x && ad.y >= ad.z) {
        v+= glm::vec3(0.0f, (d.y > 0? 1.0f : -1.0f), 0.0f); // Normal along Y
    } 
    
    if (ad.z >= ad.x && ad.z >= ad.y) {
        v+= glm::vec3(0.0f, 0.0f, (d.z > 0? 1.0f : -1.0f)); // Normal along Z
    }

    return glm::normalize(v);
}

void ensureFolderExists(const std::string& folder) {
    if (!std::filesystem::exists(folder)) {
        std::filesystem::create_directories(folder);
    }
}
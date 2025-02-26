#include "math.hpp"
#include <random>



// Compute barycentric coordinates
glm::vec3 ComputeBarycentric(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, const glm::vec3& P) {
    glm::vec3 v0 = B - A, v1 = C - A, v2 = P - A;

    float d00 = glm::dot(v0, v0);
    float d01 = glm::dot(v0, v1);
    float d11 = glm::dot(v1, v1);
    float d20 = glm::dot(v2, v0);
    float d21 = glm::dot(v2, v1);

    float denom = d00 * d11 - d01 * d01;
    
    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w; // Ensuring sum to 1

    return glm::vec3(u, v, w);
}
// Generate random float in range [0,1]
float RandomFloat() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    return dis(gen);
}
// Generate a random point inside a triangle using barycentric interpolation
glm::vec3 RandomPointInTriangle(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C) {
    float r1 = RandomFloat();
    float r2 = RandomFloat();

    // Ensure uniform distribution within the triangle
    float sqrt_r1 = std::sqrt(r1);
    float lambda1 = 1.0f - sqrt_r1;
    float lambda2 = sqrt_r1 * (1.0f - r2);
    float lambda3 = sqrt_r1 * r2;

    // Compute interpolated position
    return lambda1 * A + lambda2 * B + lambda3 * C;
}

QuadNodeInstanceBuilderHandler::QuadNodeInstanceBuilderHandler(Geometry * chunk, int * count,OctreeNode ** corners,std::vector<glm::mat4> * matrices) : QuadNodeHandler(chunk, count){
    this->corners = corners;
    this-> matrices = matrices;
}

void QuadNodeInstanceBuilderHandler::handle(OctreeNode* c0,OctreeNode* c1,OctreeNode* c2, bool sign){
    GradientPerlinSurface fps(1.0, 1.0f/128.0f, 0);

    int numPoints = 1; // Number of scattered points
    for (int i = 0; i < numPoints; i++) {
        glm::vec3 point = RandomPointInTriangle(c0->vertex.position,c1->vertex.position, c2->vertex.position);
        float perlin = fps.getHeightAt(point.x, 0, point.z);
        float hMin = 0.06;
        float hMax = 0.10;

        float force = 2.0;
        if(hMin  < perlin && perlin < hMax) {
            glm::mat4 model(1.0);
            float height = (perlin - hMin)/ (hMax-hMin);

            glm::vec3 normal = fps.getNormal(point.x, 0, point.z);

            float p = glm::dot(glm::vec3(0,1,0), normal);

            p = 1.0- glm::pow(1-p, 3);
            float h = 1.0 - glm::abs(2.0*height - 1.0);


            float deepness = Math::clamp(1.0-h, 0.0f , 1.0f);

            model = glm::translate(model, point - glm::vec3(0.0, deepness, 0.0));
            if(h*force > 1.0) {
                model = glm::scale(model, glm::vec3(1.0, h*force, 1.0));
            }     

            matrices->push_back(model);
            ++*count;
        }
    }
}

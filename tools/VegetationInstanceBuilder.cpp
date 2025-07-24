#include "tools.hpp"

#include <random>



// Compute barycentric coordinates
glm::vec3 computeBarycentric(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, const glm::vec3& P) {
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


// Generate a random point inside a triangle using barycentric interpolation
glm::vec3 randomPointInTriangle(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C) {
    float r1 = Math::randomFloat();
    float r2 = Math::randomFloat();

    // Ensure uniform distribution within the triangle
    float sqrt_r1 = std::sqrt(r1);
    float lambda1 = 1.0f - sqrt_r1;
    float lambda2 = sqrt_r1 * (1.0f - r2);
    float lambda3 = sqrt_r1 * r2;

    // Compute interpolated position
    return lambda1 * A + lambda2 * B + lambda3 * C;
}


VegetationInstanceBuilder::VegetationInstanceBuilder(long * count,std::vector<InstanceData> * instances, float pointsPerArea, float scale) : OctreeNodeTriangleHandler(count){
    this->instances = instances;
    // TODO: change to points per area
    this->pointsPerArea = pointsPerArea;
    this->scale = scale;
}

void VegetationInstanceBuilder::handle(Vertex &v0, Vertex &v1, Vertex &v2, bool sign){    
    if(v0!= v1 && v1 != v2 && v0!=v2) {
        float area = Math::triangleArea(v0.position, v1.position, v2.position);
        glm::vec3 d1 = v1.position-v0.position;
        glm::vec3 d2 = v2.position-v0.position;
        glm::vec3 n = glm::normalize(glm::cross(d1,d2));
        glm::vec3 up = glm::vec3(0,1,0);
        float p = glm::dot(up, n);

        for (int i = 0; i < pointsPerArea*area; i++) {
            glm::vec3 point = randomPointInTriangle(v0.position, v1.position, v2.position);
            
            float force = 2.0;
            //p = 1.0- glm::pow(1-p, 3);

            if(p > 0.0) {
                float height = p*force;
                float deepness = (1.0f-p);

                glm::mat4 model(1.0);
                
                if(deepness  < 1.0f) {
                    point.y -= deepness;
                }

                model = glm::translate(model, point);
                if(height > 1.0) {
                    model = glm::scale(model, scale*glm::vec3(1.0, height, 1.0));
                }     
                // model *=  Math::getRotationMatrixFromNormal(n, up);

                instances->push_back(InstanceData(0, model, deepness));
                ++*count;
            }
        }
    }

}

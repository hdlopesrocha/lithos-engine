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


VegetationInstanceBuilder::VegetationInstanceBuilder(Geometry * chunk, long * count,std::vector<InstanceData> * instances, int pointsPerTriangle) : OctreeNodeTriangleHandler(chunk, count){
    this->instances = instances;
    this->pointsPerTriangle = pointsPerTriangle;
}

void VegetationInstanceBuilder::handle(OctreeNode* c0,OctreeNode* c1,OctreeNode* c2, bool sign){
    int sizePerTile = 30;
	int tiles= 1024;
	int height = 2048;
    GradientPerlinSurface fps(height, 1.0/(256.0f*sizePerTile), -64);
    
    if(c0 != NULL && c1 != NULL && c2!=NULL) {
        Vertex v0 = c0->vertex;
        Vertex v1 = c1->vertex;
        Vertex v2 = c2->vertex;

        if(c0!= c1 && c1 != c2 && c0!=c2) {
            for (int i = 0; i < pointsPerTriangle; i++) {
                glm::vec3 point = randomPointInTriangle(v0.position, v1.position, v2.position);
                float perlin = fps.getHeightAt(point.x, point.z);
                float hMin = 0.06;
                float hMax = 0.10;

                float force = 2.0;
                if(hMin  < perlin && perlin < hMax) {
                    float height = (perlin - hMin)/ (hMax-hMin);

                    glm::vec3 normal = fps.getNormal(point.x, 0, point.z);

                    float p = glm::dot(glm::vec3(0,1,0), normal);

                    p = 1.0- glm::pow(1-p, 3);
                    float h = 1.0 - glm::abs(2.0*height - 1.0);


                    float deepness = Math::clamp(1.0-h, 0.0f , 1.0f);

                    glm::mat4 model(1.0);
                    model = glm::translate(model, point);
                    if(h*force > 1.0) {
                        model = glm::scale(model, glm::vec3(1.0, h*force, 1.0));
                    }     
                    model *=  Math::getRotationMatrixFromNormal(v0.normal, glm::vec3(0.0,1.0,0.0));

                    instances->push_back(InstanceData(0, model, deepness));
                    ++*count;
                }
            }
        }
    }
}

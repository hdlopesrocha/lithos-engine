#include "math.hpp"

void ComputeTangent(
    const glm::vec3& pos1, const glm::vec3& pos2, const glm::vec3& pos3, // Vertex positions
    const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3,    // Texture coordinates
    glm::vec4& tangent, glm::vec4& bitangent) 
{
    // Compute edge vectors
    glm::vec3 edge1 = pos2 - pos1;
    glm::vec3 edge2 = pos3 - pos1;

    // Compute delta UV vectors
    glm::vec2 deltaUV1 = uv2 - uv1;
    glm::vec2 deltaUV2 = uv3 - uv1;

    // Compute inverse determinant
    const float EPSILON = 1e-6f;
    float determinant = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
    float f = (abs(determinant) > EPSILON) ? (1.0f / determinant) : 0.0f;

    // Compute tangent and bitangent as vec3
    glm::vec3 T = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
    glm::vec3 B = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);

    // Normalize results
    T = glm::normalize(T);
    B = glm::normalize(B);

    // Compute handedness (TBN orientation)
    float handedness = (glm::dot(glm::cross(T, B), glm::normalize(edge1)) < 0.0f) ? -1.0f : 1.0f;

    // Assign to vec4 (handedness in w component)
    tangent = glm::vec4(T, handedness);
    bitangent = glm::vec4(B, 0.0f); // Bitangent doesn't need handedness
}


int triplanarPlane(glm::vec3 position, glm::vec3 normal) {
    glm::vec3 absNormal = glm::abs(normal);
    if (absNormal.x > absNormal.y && absNormal.x > absNormal.z) {
        return normal.x > 0 ? 0 : 1;
    } else if (absNormal.y > absNormal.x && absNormal.y > absNormal.z) {
        return normal.y > 0 ? 2 : 3;
    } else {
        return normal.z > 0 ? 4 : 5;
    }
}

glm::vec2 triplanarMapping(glm::vec3 position, int plane) {
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

int addTriangle(OctreeNode* c0, OctreeNode* c1, OctreeNode* c2, Geometry * chunk, bool reverse) {
   

    int count = 0;
    if(c0 != NULL && c1 != NULL && c2!=NULL) {
        Vertex v0 = c0->vertex;
        Vertex v1 = c1->vertex;
        Vertex v2 = c2->vertex;

        if(c0!= c1 && c1 != c2 && c0!=c2 && c0->vertex.brushIndex>=0 && c1->vertex.brushIndex>=0 && c2->vertex.brushIndex>=0){

            int plane = triplanarPlane(v0.position, v0.normal);//TODO calculate normal from positions
            v0.texCoord = triplanarMapping(v0.position, plane) * 0.1f;
            v1.texCoord = triplanarMapping(v1.position, plane) * 0.1f;
            v2.texCoord = triplanarMapping(v2.position, plane) * 0.1f;
            
            glm::vec4 bitangent;
            glm::vec4 tangent;
            ComputeTangent(v0.position, v1.position, v2.position, v0.texCoord, v1.texCoord, v2.texCoord, tangent, bitangent);
            v2.tangent = v1.tangent = v0.tangent;
        
            chunk->addVertex(reverse ? v2 : v0);
            chunk->addVertex(reverse ? v1 : v1);
            chunk->addVertex(reverse ? v0 : v2);
            ++count;
        }
    }
    return count;
}

OctreeNodeTriangleTesselator::OctreeNodeTriangleTesselator(Geometry * chunk, int * count) : OctreeNodeTriangleHandler(chunk, count){

}

void OctreeNodeTriangleTesselator::handle(OctreeNode* c0,OctreeNode* c1,OctreeNode* c2, bool sign) {
    if(c0 != NULL && c1 != NULL && c2!=NULL) {
	    *count += addTriangle(c0,c1,c2, chunk, sign);
    }
}

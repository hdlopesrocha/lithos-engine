#include "math.hpp"


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

int addTriangle(OctreeNode* c0, OctreeNode* c1, OctreeNode* c2, Geometry * chunk, bool reverse, bool triplanar) {
   

    int count = 0;
    if(c0 != NULL && c1 != NULL && c2!=NULL) {
        Vertex v0 = c0->vertex;
        Vertex v1 = c1->vertex;
        Vertex v2 = c2->vertex;

        if(c0!= c1 && c1 != c2 && c0!=c2 && c0->vertex.brushIndex>=0 && c1->vertex.brushIndex>=0 && c2->vertex.brushIndex>=0){

            if(triplanar) {
                int plane = triplanarPlane(v0.position, v0.normal);//TODO calculate normal from positions
                v0.texCoord = triplanarMapping(v0.position, plane) * 0.1f;
                v1.texCoord = triplanarMapping(v1.position, plane) * 0.1f;
                v2.texCoord = triplanarMapping(v2.position, plane) * 0.1f;
            }
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
	    *count += addTriangle(c0,c1,c2, chunk, sign, false); //triplanar in GPU
    }
}

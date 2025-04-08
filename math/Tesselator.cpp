#include "math.hpp"


Tesselator::Tesselator(Octree * tree, Geometry * chunk, long * count): OctreeNodeTriangleHandler(chunk, count) {
	this->tree = tree;
	this->triangles = 0;
}

void Tesselator::before(OctreeNodeData &params) {		
    if(params.node->isEmpty()) {		
        // Must go to zero and try to create triangles in the border with highest detail
        // If the node is simplified then the triangles could have equal vertices
        // If a triangle as 2 equal vertices, it's a line, this triangle will be ignored
        // The same happens when iterating fully inside a simplified chunk, that triangle will be a single point
		tree->handleQuadNodes(params , this, true);
	}
}

void Tesselator::after(OctreeNodeData &params) {
	return;
}

bool Tesselator::test(OctreeNodeData &params) {			
	return params.node->solid != ContainmentType::Contains;
}

void Tesselator::getOrder(OctreeNodeData &params, int * order){
	for(int i = 7 ; i >= 0 ; --i) {
		order[i] = i;
	}
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

int addTriangle(OctreeNode* c0, OctreeNode* c1, OctreeNode* c2, Geometry * chunk, bool reverse, bool triplanar, float triplanarScale) {
   

    int count = 0;
    if(c0 != NULL && c1 != NULL && c2!=NULL) {
        Vertex v0 = c0->vertex;
        Vertex v1 = c1->vertex;
        Vertex v2 = c2->vertex;

        if(c0!= c1 && c1 != c2 && c0!=c2 && 
            c0->vertex.brushIndex>DISCARD_BRUSH_INDEX && 
            c1->vertex.brushIndex>DISCARD_BRUSH_INDEX && 
            c2->vertex.brushIndex>DISCARD_BRUSH_INDEX){
            glm::vec3 d1 = v1.position-v0.position;
            glm::vec3 d2 = v2.position-v0.position;
            glm::vec3 n = glm::cross(d2,d1);

            if(triplanar) {
                int plane = triplanarPlane(v0.position, n);
                v0.texCoord = triplanarMapping(v0.position, plane)*triplanarScale;
                v1.texCoord = triplanarMapping(v1.position, plane)*triplanarScale;
                v2.texCoord = triplanarMapping(v2.position, plane)*triplanarScale;
            }
            chunk->addVertex(reverse ? v2 : v0);
            chunk->addVertex(reverse ? v1 : v1);
            chunk->addVertex(reverse ? v0 : v2);
            ++count;
        }
    }
    return count;
}


void Tesselator::handle(OctreeNode* c0,OctreeNode* c1,OctreeNode* c2, bool sign) {
    if(c0 != NULL && c1 != NULL && c2!=NULL) {
	    *count += addTriangle(c0,c1,c2, chunk, sign, true, 0.02f); //triplanar in GPU
    }
}

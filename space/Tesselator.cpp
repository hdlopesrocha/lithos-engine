#include "space.hpp"


Tesselator::Tesselator(Octree * tree, Geometry * chunk, long * count): OctreeNodeTriangleHandler(count) {
	this->tree = tree;
    this->chunk = chunk;
}

void Tesselator::virtualize(OctreeNodeData &params, int levels) {
    Vertex v = params.node->vertex;
    Plane p(v.normal, v.position);

    if(levels > 0) {
        for(int i = 7 ; i >= 0 ; --i) {
            BoundingCube c = params.cube.getChild(i);
            //if(p.test(c) != ContainmentType::Disjoint) {
                OctreeNodeData p(params);
                p.cube = c;
                p.level = params.level + 1;
                virtualize(p, levels - 1);
            //}
        }
    } else {
		tree->handleQuadNodes(params , this, true);
    }
}

void Tesselator::before(OctreeNodeData &params) {		
    /*if(params.node->isSimplified()) {
        int levels = tree->getMaxLevel(params.node, 0);
		virtualize(params, levels);
    	//tree->handleQuadNodes(params , this, true);
	}*/
    if(params.node->isLeaf()) {
       	tree->handleQuadNodes(params , this, true);
    }
}

void Tesselator::after(OctreeNodeData &params) {
	return;
}

bool Tesselator::test(OctreeNodeData &params) {			
	return !params.node->isSolid() && !params.node->isLeaf();
}

void Tesselator::getOrder(OctreeNodeData &params, uint8_t * order){
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
	    *count += addTriangle(c0,c1,c2, chunk, sign, true, 0.2f); //triplanar in GPU
    }
}

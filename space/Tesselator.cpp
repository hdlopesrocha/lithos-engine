#include "space.hpp"


Tesselator::Tesselator(Geometry * chunk, long * count, ChunkContext * context): OctreeNodeTriangleHandler(count), context(context) {
    this->chunk = chunk;
}

void Tesselator::virtualize(Octree * tree, OctreeNodeData &data, uint levels) {
    if(data.level >= levels) {
        tree->handleQuadNodes(data, this, true, context);
    } else {
        for(int i = 0 ; i < 8 ; ++i) {
            OctreeNodeData childData(data.level + 1, data.node, data.cube.getChild(i), data.context);
            virtualize(tree, childData, levels);
        }
    }
}

void Tesselator::before(Octree * tree, OctreeNodeData &params) {		
    if(params.node->isLeaf() && !params.node->isSolid() && !params.node->isEmpty()) {
        uint levels = tree->getMaxLevel(params.cube);
        if(params.level < levels) {
            std::cout << "Virtualize at level " << params.level << "/" << levels << std::endl;
        }
        virtualize(tree, params, levels);
	}
}

void Tesselator::after(Octree * tree, OctreeNodeData &params) {
	return;
}

bool Tesselator::test(Octree * tree, OctreeNodeData &params) {			
	return !params.node->isSolid() && !params.node->isLeaf();
}

void Tesselator::getOrder(Octree * tree, OctreeNodeData &params, uint8_t * order){
	for(int i = 7 ; i >= 0 ; --i) {
		order[i] = i;
	}
}


int triplanarPlane(glm::vec3 normal) {
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

void Tesselator::handle(Vertex &v0, Vertex &v1, Vertex &v2, bool reverse) {
    if(v0.brushIndex>DISCARD_BRUSH_INDEX && 
        v1.brushIndex>DISCARD_BRUSH_INDEX && 
        v2.brushIndex>DISCARD_BRUSH_INDEX) {

        bool triplanar = true;
        float triplanarScale = 0.1f;
        glm::vec3 d1 = v1.position -v0.position;
        glm::vec3 d2 = v2.position -v0.position;
        glm::vec3 n = glm::cross(d2,d1);


        if(triplanar) {
            int plane = triplanarPlane(n);
            v0.texCoord = triplanarMapping(v0.position, plane)*triplanarScale;
            v1.texCoord = triplanarMapping(v1.position, plane)*triplanarScale;
            v2.texCoord = triplanarMapping(v2.position, plane)*triplanarScale;
        }

        chunk->addVertex(reverse ? v2 : v0);
        chunk->addVertex(reverse ? v1 : v1);
        chunk->addVertex(reverse ? v0 : v2);
        ++(*count);
    }
}

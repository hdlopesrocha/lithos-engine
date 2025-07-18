#version 460

#define SpaceType_Empty 0
#define SpaceType_Surface 1
#define SpaceType_Solid 2

struct OctreeSerialized {
    vec3 min;
    float length;
    float chunkSize;
};


struct OctreeNodeSerialized {
	float sdf[8];
    uint children[8];
    int brushIndex;
    uint bits;
};

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texCoord;
    uint brushIndex;
};

struct ComputeShaderResult {
    vec4 result4f; 
    uint vertexCount;
    uint indexCount;
};

const ivec4 tessOrder[3] = ivec4[](
    ivec4(0, 1, 3, 2),
    ivec4(0, 2, 6, 4),
    ivec4(0, 4, 5, 1)
);

const ivec2 tessEdge[3] = ivec2[](
    ivec2(3, 7),
    ivec2(6, 7),
    ivec2(5, 7)
);


const ivec2 SDF_EDGES[12] = ivec2[](
    ivec2(0, 1), 
    ivec2(1, 3), 
    ivec2(3, 2), 
    ivec2(2, 0),
    ivec2(4, 5),
    ivec2(5, 7), 
    ivec2(7, 6), 
    ivec2(6, 4),
    ivec2(0, 4), 
    ivec2(1, 5), 
    ivec2(2, 6), 
    ivec2(3, 7)
); 



layout(local_size_x = 64) in;

layout(std430, binding = 0) buffer VertexBuffer {
    Vertex vertices[];
};

layout(std430, binding = 1) buffer IndexBuffer {
    uint indices[];
};

layout(std140, binding = 2) buffer ComputeShaderResultBuffer {
    ComputeShaderResult result;
};

layout(std430, binding = 3) readonly buffer OctreeBuffer {
    vec3 min;
    float length;
    float chunkSize;
};

layout(std430, binding = 4) readonly buffer NodesBuffer {
    OctreeNodeSerialized nodes[];
};

vec2 triplanarMapping(vec3 position, int plane) {
    switch (plane) {
        case 0: return vec2(-position.z, -position.y);
        case 1: return vec2(position.z, -position.y);
        case 2: return vec2(position.x, position.z);
        case 3: return vec2(position.x, -position.z);
        case 4: return vec2(position.x, -position.y);
        case 5: return vec2(-position.x, -position.y);
        default: return vec2(0.0,0.0);
    }
}

vec3 getShift(int i) {
    return vec3(
        (i & 4) != 0 ? 1.0 : 0.0,
        (i & 2) != 0 ? 1.0 : 0.0,
        (i & 1) != 0 ? 1.0 : 0.0
    );
}
bool contains(vec3 pos) {
    vec3 max = min + vec3(length);
    return all(greaterThanEqual(pos, min)) && all(lessThanEqual(pos, max));
}

int getNodeIndex(vec3 pos, vec3 cubeMin, float cubeLength) {
    vec3 center = cubeMin + vec3(cubeLength * 0.5);
    int idx = 0;
    if (pos.x >= center.x) idx |= 4;
    if (pos.y >= center.y) idx |= 2;
    if (pos.z >= center.z) idx |= 1;
    return idx;
}
bool isSimplified(uint bits) {
    return (bits & (0x1u << 3)) != 0;  // exemplo: simplification flag no bit 0
}


// Obtem o índice do nó que contém a posição
int getNodeAt(vec3 pos, bool simplification) {
    if (!contains(pos)) return -1; // Não está na octree

    int nodeIdx = 0; // root
    vec3 cubeMin = min;
    float cubeLength = length;

    while (true) {
        OctreeNodeSerialized node = nodes[nodeIdx];
        if (simplification && isSimplified(node.bits)) {
            break;
        }

        int childIdx = getNodeIndex(pos, cubeMin, cubeLength);
        uint nextNodeIdx = node.children[childIdx];

        if (nextNodeIdx == 0) break;  // Sem filho → este é o nó final

        nodeIdx = int(nextNodeIdx);
        cubeLength *= 0.5;
        if ((childIdx & 4) != 0) cubeMin.x += cubeLength;
        if ((childIdx & 2) != 0) cubeMin.y += cubeLength;
        if ((childIdx & 1) != 0) cubeMin.z += cubeLength;
    }

    return nodeIdx;
}

// Simple helper to check if node is a leaf (no children)
bool isLeaf(OctreeNodeSerialized node) {
    for (int i = 0; i < 8; i++) {
        if (node.children[i] != 0)
            return false;
    }
    return true;
}

// Estimate normal from SDF
vec3 estimateNormal(float sdf[8]) {
    // Central differences across cube edges
    vec3 grad = vec3(
        sdf[1] - sdf[0] + sdf[3] - sdf[2] + sdf[5] - sdf[4] + sdf[7] - sdf[6],
        sdf[2] - sdf[0] + sdf[3] - sdf[1] + sdf[6] - sdf[4] + sdf[7] - sdf[5],
        sdf[4] - sdf[0] + sdf[5] - sdf[1] + sdf[6] - sdf[2] + sdf[7] - sdf[3]
    );
    return normalize(grad);
}

int evalSDF(float sdf[8]) {
    bool hasPositive = false;
    bool hasNegative = false;
    for (int i = 0; i < 8; ++i) {  
        if (sdf[i] >= 0.0f) {
            hasPositive = true;
        } else {
            hasNegative = true;
        }
        if(hasPositive && hasNegative) {
            break;  // Early exit if both signs are found
        }
    }
    return hasNegative && hasPositive ? SpaceType_Surface : (hasPositive ? SpaceType_Empty : SpaceType_Solid);
}

vec3 estimatePosition(float sdf[8]) {
    // Early exit if there's no surface inside this cube
    vec3 center =min+ vec3(length*0.5f);
    int eval = evalSDF(sdf);
    if(eval != SpaceType_Surface) {
        return center;  // or some fallback value
    }
    vec3 sum = vec3(0.0f);
    int sums = 0;
    for (int i = 0; i < 12; ++i) {
        ivec2 edge = SDF_EDGES[i];

        float d0 = sdf[edge[0]];
        float d1 = sdf[edge[1]];


		bool sign0 = d0 < 0.0f;
		bool sign1 = d1 < 0.0f;

        if (sign0 != sign1) {
            vec3 p0 = min + length * getShift(edge[0]);
            vec3 p1 = min + length * getShift(edge[1]);
            float t = d0 / (d0 - d1);  // Safe due to sign change
            sum+= p0 + t * (p1 - p0);
            ++sums;
        }
    }
    if(sums > 0) {
        return sum / sums;
    }
    else {
        return center;
    }
}


void createVertex(Vertex vertex) {
// Generate vertex
    uint vertexIdx = atomicAdd(result.vertexCount, 1);
    vertices[vertexIdx] = vertex;
    uint indexIdx = atomicAdd(result.indexCount, 1);
    indices[indexIdx] = vertexIdx;
}   

bool mustSkip(OctreeNodeSerialized node) {
    if (!isLeaf(node))
        return true;

    bool allPositive = true;
    bool allNegative = true;
    for (int i = 0; i < 8; i++) {
        if (node.sdf[i] <= 0.0) allPositive = false;
        if (node.sdf[i] >= 0.0) allNegative = false;
    }
    // If all are positive or all are negative, skip (no surface)
    return allPositive || allNegative;
}



int triplanarPlane(vec3 position, vec3 normal) {
    vec3 absNormal = abs(normal);
    if (absNormal.x > absNormal.y && absNormal.x > absNormal.z) {
        return normal.x > 0.0f ? 0 : 1;
    } else if (absNormal.y > absNormal.x && absNormal.y > absNormal.z) {
        return normal.y > 0.0f ? 2 : 3;
    } else {
        return normal.z > 0.0f ? 4 : 5;
    }
}

void handleTriangle(OctreeNodeSerialized n0, OctreeNodeSerialized n1 , OctreeNodeSerialized n2, bool sign) {
    
    float triplanarScale = 1.0f;
    
    if(n0.brushIndex < 0 || n1.brushIndex < 0 || n2.brushIndex < 0 ) {
        return;
    }

    result.result4f= vec4(n0.sdf[0],n0.sdf[1],n0.sdf[2],n0.sdf[3]);

    // uint indexIdx = atomicAdd(indexCount, 1);


    vec3 p0 = estimatePosition(n0.sdf);
    vec3 p1 = estimatePosition(n1.sdf);
    vec3 p2 = estimatePosition(n2.sdf);

    vec3 d1 = p1 - p0;
    vec3 d2 = p2 - p0;
    vec3 n = cross(d2,d1);
    int plane = triplanarPlane(p0, n);

    Vertex v0;
    v0.position = p0;
    v0.normal = estimateNormal(n0.sdf);
    v0.texCoord = triplanarMapping(p0, plane)*triplanarScale;
    v0.brushIndex = n0.brushIndex;

    Vertex v1;
    v1.position = p1;
    v1.normal = estimateNormal(n1.sdf);
    v1.texCoord = triplanarMapping(p1, plane)*triplanarScale;
    v1.brushIndex = n1.brushIndex;

    Vertex v2;
    v2.position = p2;
    v2.normal = estimateNormal(n2.sdf);
    v2.texCoord = triplanarMapping(p2, plane)*triplanarScale;
    v2.brushIndex = n2.brushIndex;

    createVertex(sign ? v2 : v0);
    createVertex(v1);
    createVertex(sign ? v0 : v2);

}


void main() {
    uint nodeIdx = gl_GlobalInvocationID.x;
    if (nodeIdx >= nodes.length()) return;
    
    OctreeNodeSerialized node = nodes[nodeIdx];

    if(mustSkip(node)){
        return;
    }

    for(int k =0 ; k < tessEdge.length(); ++k) {
		ivec2 edge = tessEdge[k];

        float d0 = node.sdf[edge[0]];
        float d1 = node.sdf[edge[1]];

		bool sign0 = d0 < 0.0f;
		bool sign1 = d1 < 0.0f;

		if(sign0 != sign1) {
			ivec4 quad = tessOrder[k];
            OctreeNodeSerialized quads[4];
            int direction = 1;
			for(int i =0; i<4 ; ++i) {
                vec3 cubeCenter = min + vec3(length*0.25);
                vec3 pos = cubeCenter + direction * vec3(length*0.5) * getShift(i);

				int n = getNodeAt(pos, false);
                if(n >= 0) {
                    quads[i] = nodes[n];
                }else {
                    quads[i].brushIndex = -1; // Mark as empty and solid
                }
			}

            handleTriangle(quads[0],quads[2],quads[1], sign1);
            handleTriangle(quads[0],quads[3],quads[2], sign1);

		}

    }    
}
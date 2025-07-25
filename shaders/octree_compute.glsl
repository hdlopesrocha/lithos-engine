#version 460


#define SpaceType_Empty 0
#define SpaceType_Surface 1
#define SpaceType_Solid 2

#include<structs.glsl>

struct OctreeNodeCubeSerialized {
	float sdf[8];
    uint children[8];
    vec3 min;
    int brushIndex;
    vec3 length;
    uint bits;
};

struct ComputeShaderOutput {
    vec4 result4f0; 
    vec4 result4f1; 
    uint vertexCount;
    uint indexCount;
};

struct ComputeShaderInput {
    vec4 chunkMin;
    vec4 chunkLength;
};

layout(local_size_x = 64) in;

layout(std430, binding = 0) buffer VertexBuffer {
    Vertex vertices[];
};

layout(std430, binding = 1) buffer IndexBuffer {
    uint indices[];
};

layout(std430, binding = 2) buffer InstanceBuffer {
    InstanceData instance;
};

layout(std430, binding = 3) buffer OutputBuffer {
    ComputeShaderOutput shaderOutput;
};

layout(std430, binding = 4) readonly buffer NodesBuffer {
    OctreeNodeCubeSerialized nodes[];
};

layout(std430, binding = 5) readonly buffer InputBuffer {
    ComputeShaderInput shaderInput;
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
    return vec3(i >> 2 & 1, i >> 1 & 1, i & 1);
}

bool contains(vec3 sourceMin, vec3 sourceLength, vec3 targetMin, vec3 targetLength) {
    vec3 sourceMax = sourceMin + sourceLength;
    vec3 targetMax = targetMin + targetLength;
    return all(greaterThanEqual(targetMin, sourceMin)) && all(lessThanEqual(targetMax, sourceMax));
}

bool contains(vec3 min, vec3 length, vec3 pos) {
    vec3 max = min + length;
    return all(greaterThanEqual(pos, min)) && all(lessThanEqual(pos, max));
}

int getNodeIndex(vec3 pos, vec3 cubeMin, vec3 cubeLength) {
	vec3 c = cubeMin + cubeLength * 0.5;
    return (pos.x >= c.x ? 4 : 0) + (pos.y >= c.y ? 2 : 0) + (pos.z >= c.z ? 1 : 0);
}

bool isSimplified(uint bits) {
    return (bits & (0x1u << 3)) != 0u;  // exemplo: simplification flag no bit 0
}

bool isSolid(uint bits) {
    return (bits & (0x1u << 0)) != 0u;  // exemplo: simplification flag no bit 0
}

bool isEmpty(uint bits) {
    return (bits & (0x1u << 1)) != 0u;  // exemplo: simplification flag no bit 0
}


// Obtem o índice do nó que contém a posição
int getNodeAt(vec3 pos, bool simplification) {
    OctreeNodeCubeSerialized root = nodes[0];
    if (!contains(root.min, root.length, pos)) return -1; // Não está na octree

    uint nodeIdx = 0; // root
    vec3 cubeMin = root.min;
    vec3 cubeLength = root.length;

    while (true) {
        OctreeNodeCubeSerialized node = nodes[nodeIdx];
        if (simplification && isSimplified(node.bits)) {
            break;
        }

        int childIdx = getNodeIndex(pos, cubeMin, cubeLength);
        uint nextNodeIdx = node.children[childIdx];

        if (nextNodeIdx == 0u || nextNodeIdx >= nodes.length()) break;

        nodeIdx = nextNodeIdx;
        cubeLength *= 0.5;
        cubeMin += getShift(childIdx) * cubeLength;
    }

    return int(nodeIdx);
}

// Simple helper to check if node is a leaf (no children)
bool isLeaf(OctreeNodeCubeSerialized node) {
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

vec3 estimatePosition(float sdf[8], vec3 min, vec3 length) {
    // Early exit if there's no surface inside this cube
    vec3 center =min+ length*0.5f;
//    return center;  // or some fallback value
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

void createVertex(Vertex vertex, uint vertexIdx, uint indexIdx) {
    vertices[vertexIdx] = vertex;
    indices[indexIdx] = vertexIdx;
}   

bool isSurface(OctreeNodeCubeSerialized node) {
    bool hasPositive = false;
    bool hasNegative = false;
    for (int i = 0; i < 8; i++) {
        if (node.sdf[i] >= 0.0) {
            hasPositive = true;
        }else {
            hasNegative = true;
        }
    }
    // If all are positive or all are negative, skip (no surface)
    return (hasPositive && hasNegative);
}

int triplanarPlane(vec3 normal) {
    vec3 absNormal = abs(normal);
    if (absNormal.x > absNormal.y && absNormal.x > absNormal.z) {
        return normal.x > 0.0f ? 0 : 1;
    } else if (absNormal.y > absNormal.x && absNormal.y > absNormal.z) {
        return normal.y > 0.0f ? 2 : 3;
    } else {
        return normal.z > 0.0f ? 4 : 5;
    }
}

void handleTriangle(Vertex v0, Vertex v1 , Vertex v2, bool sign) { 
    if(v0.brushIndex < 0 || v1.brushIndex < 0 || v2.brushIndex < 0) {
        return; // Skip if any vertex is empty or solid
    }
    //shaderOutput.result4f1= vec4(v0.position.xyz, v0.brushIndex);
    float triplanarScale = 0.1f;

    vec3 d1 = v1.position.xyz - v0.position.xyz;
    vec3 d2 = v2.position.xyz - v0.position.xyz;
    vec3 n = cross(d2,d1);
    int plane = triplanarPlane(n);
    v0.texCoord = triplanarMapping(v0.position.xyz , plane)*triplanarScale;
    v1.texCoord = triplanarMapping(v1.position.xyz , plane)*triplanarScale;
    v2.texCoord = triplanarMapping(v2.position.xyz , plane)*triplanarScale;

    // Generate vertex
    uint vertexIdx = atomicAdd(shaderOutput.vertexCount, 3);
    uint indexIdx = atomicAdd(shaderOutput.indexCount, 3);

    // CASE 1: This shows nothing
    createVertex(sign ? v2 : v0,    vertexIdx+0, indexIdx+0);
    createVertex(v1,                vertexIdx+1, indexIdx+1);
    createVertex(sign ? v0 : v2,    vertexIdx+2, indexIdx+2);

}


void main() {
    uint nodeIdx = gl_GlobalInvocationID.x;
    if (nodeIdx >= nodes.length()) return;
   
    OctreeNodeCubeSerialized node = nodes[nodeIdx];

    if(!contains(shaderInput.chunkMin.xyz, shaderInput.chunkLength.xyz, node.min, node.length) 
        || !isLeaf(node)
        || !isSurface(node)) {
        return;
    }

    shaderOutput.result4f0= vec4(nodes[0].min, 0.0f);
    shaderOutput.result4f1= vec4(nodes[0].length, 0.0f);

    for(int k =0 ; k < tessEdge.length(); ++k) {
		ivec2 edge = tessEdge[k];

        float d0 = node.sdf[edge[0]];
        float d1 = node.sdf[edge[1]];

		bool sign0 = d0 < 0.0f;
		bool sign1 = d1 < 0.0f;

		if(sign0 != sign1) {
			ivec4 order = tessOrder[k];
            Vertex vertices[4];
            int direction = 1;
			for(int i =0; i<4 ; ++i) {
                vec3 cubeCenter = node.min + node.length*0.5;
                vec3 pos = cubeCenter + direction * node.length* getShift(order[i]);

				int n = getNodeAt(pos, false);
                OctreeNodeCubeSerialized quadNode;
                
                if(n >= 0) {
                    quadNode = nodes[n];
                }

                if(n >= 0 && !isSolid(quadNode.bits) && !isEmpty(quadNode.bits)) {
                    Vertex vertex;
                    vertex.brushIndex = quadNode.brushIndex;
                    vertex.position = vec4(estimatePosition(quadNode.sdf, quadNode.min, quadNode.length), 0.0f);
                    vertex.normal = vec4(estimateNormal(quadNode.sdf), 0.0f);
                    vertices[i] = vertex;
                } else {
                    vertices[i].brushIndex = -1; // Mark as empty and solid
                }
			}
            handleTriangle(vertices[0], vertices[2], vertices[1], sign1);
            handleTriangle(vertices[0], vertices[3], vertices[2], sign1);
		}

    }    
}
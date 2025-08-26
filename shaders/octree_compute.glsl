#version 460


#define SpaceType_Empty 0
#define SpaceType_Surface 1
#define SpaceType_Solid 2
#define DISCARD_BRUSH_INDEX -1

#include<structs.glsl>

struct OctreeNodeCubeSerialized {
    vec4 position;
    vec4 normal;
    vec2 texCoord;
    int brushIndex;
    uint sign;
    uint children[8];
    vec3 min;
    uint bits;
    vec3 length;
    uint level;
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


const ivec4 TESSELATION_ORDERS[3] = ivec4[](
    ivec4(0, 1, 3, 2),
    ivec4(0, 2, 6, 4),
    ivec4(0, 4, 5, 1)
);

const ivec2 TESSELATION_EDGES[3] = ivec2[](
    ivec2(3, 7),
    ivec2(6, 7),
    ivec2(5, 7)
);

const vec3 CUBE_CORNERS[8] = vec3[](
    vec3(0,0,0),
    vec3(0,0,1),
    vec3(0,1,0),
    vec3(0,1,1),
    vec3(1,0,0),
    vec3(1,0,1),
    vec3(1,1,0),
    vec3(1,1,1)
);

int triplanarPlane(vec3 normal) {
    vec3 absNormal = abs(normal);
    if (absNormal.x > absNormal.y && absNormal.x > absNormal.z) {
        return 0;
    } else if (absNormal.y > absNormal.x && absNormal.y > absNormal.z) {
        return 1;
    } else {
        return 2;
    }
}

vec2 triplanarMapping(vec3 position, int plane) {
    switch (plane) {
        case 0: return vec2(-position.z, -position.y);
        case 1: return vec2(position.x, position.z);
        case 2: return vec2(position.x, -position.y);
        default: return vec2(0.0,0.0);
    }
}

vec3 getShift(int i) {
    return CUBE_CORNERS[i];
}

vec3 getCorner(int i, vec3 min, vec3 length) {
    return min + getShift(i) * length;
}


bool contains(vec3 sourceMin, vec3 sourceLength, vec3 targetMin, vec3 targetLength) {
    vec3 sourceMax = sourceMin + sourceLength;
    vec3 targetMax = targetMin + targetLength;
    return all(greaterThanEqual(targetMin, sourceMin)) && all(lessThanEqual(targetMax, sourceMax));
}

bool intersects(vec3 sourceMin, vec3 sourceLength, vec3 targetMin, vec3 targetLength) {
    vec3 sourceMax = sourceMin + sourceLength;
    vec3 targetMax = targetMin + targetLength;
    return all(lessThanEqual(sourceMin, targetMax)) && all(greaterThanEqual(sourceMax, targetMin));
}

bool contains(vec3 min, vec3 length, vec3 pos) {
    vec3 max = min + length;
    return all(greaterThanEqual(pos, min)) && all(lessThanEqual(pos, max));
}

int getNodeIndex(vec3 pos, vec3 cubeMin, vec3 cubeLength) {
	vec3 c = cubeMin + cubeLength * 0.5;
    return (pos.x >= c.x ? 4 : 0) + (pos.y >= c.y ? 2 : 0) + (pos.z >= c.z ? 1 : 0);
}

bool isSolid(uint bits) {
    return (bits & (0x1u << 0)) != 0u; 
}

bool isEmpty(uint bits) {
    return (bits & (0x1u << 1)) != 0u;  
}

bool isSimplified(uint bits) {
    return (bits & (0x1u << 2)) != 0u; 
}
bool isLeaf(uint bits) {
    return (bits & (0x1u << 5)) != 0u; 
}

// Obtem o índice do nó que contém a posição
int getNodeAt(vec3 pos, uint level, bool simplification) {
    uint nodeIdx = 0; // root
    OctreeNodeCubeSerialized node = nodes[nodeIdx];
    if (!contains(node.min, node.length, pos)){
        return DISCARD_BRUSH_INDEX; // Não está na octree
    } 

    vec3 cubeMin = node.min;
    vec3 cubeLength = node.length;

    for (; level > 0; --level) {
        if (simplification && isSimplified(node.bits)) {
            break;
        }
        if(isSolid(node.bits) && isEmpty(node.bits)){
            return DISCARD_BRUSH_INDEX;
        }

        int i = getNodeIndex(pos, cubeMin, cubeLength);
        cubeLength *= 0.5f;
        cubeMin += cubeLength * getShift(i);

        nodeIdx = node.children[i];
        if (nodeIdx == 0u) {
            return DISCARD_BRUSH_INDEX;
        } 
        node = nodes[nodeIdx];
    }

    return int(nodeIdx);
}

vec3 interpolate(vec3 p0, vec3 p1, float v0, float v1) {
    float t = v0 / (v0 - v1);  // Linear interpolation weight
    return mix(p0, p1, t);     // Equivalent to: p0 + t * (p1 - p0)
}


void createVertex(Vertex vertex, uint vertexIdx, uint indexIdx) {
    vertices[vertexIdx] = vertex;
    indices[indexIdx] = vertexIdx;
}   

float triangleArea(vec3 a, vec3 b, vec3 c) {
    vec3 ab = b - a;
    vec3 ac = c - a;
    return length(cross(ab, ac)) * 0.5;
}

void handleTriangle(Vertex v0, uint i0, Vertex v1, uint i1, Vertex v2, uint i2, bool sign) { 
    if(v0.brushIndex == DISCARD_BRUSH_INDEX 
        || v1.brushIndex == DISCARD_BRUSH_INDEX 
        || v2.brushIndex == DISCARD_BRUSH_INDEX
        || triangleArea(v0.position.xyz, v1.position.xyz, v2.position.xyz) < 0.00001f) {
        return; 
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
    uint indexIdx = atomicAdd(shaderOutput.indexCount, 3);

    Vertex va, vb, vc;
    uint ia, ib, ic;

    vb = v1;
    ib = i1;
    if(sign) {
        va = v2;
        vc = v0;
        ia = i2;
        ic = i0;
    } else {
        va = v0;
        vc = v2;
        ia = i0;
        ic = i2;
    }

    createVertex(va, 3*ia + plane, indexIdx+0);
    createVertex(vb, 3*ib + plane, indexIdx+1);
    createVertex(vc, 3*ic + plane, indexIdx+2);
}

void main() {
    uint nodeIdx = gl_GlobalInvocationID.x;
    if (nodeIdx >= nodes.length()) return;
   
    OctreeNodeCubeSerialized node = nodes[nodeIdx];

    if(!contains(shaderInput.chunkMin.xyz, shaderInput.chunkLength.xyz, node.min, node.length) 
        || !isLeaf(node.bits)) {
        return;
    }

    //shaderOutput.result4f0 = shaderInput.chunkMin;
    //shaderOutput.result4f1 = shaderInput.chunkLength;

    vec3 cubeCenter = node.min + node.length * 0.5f;    
    int neighbors[8];
    for(int i = 0 ; i < 8; ++i) {
        vec3 pos = cubeCenter + node.length * getShift(i);
        neighbors[i] = getNodeAt(pos, node.level, true);
    }

    for(int k = 0 ; k < TESSELATION_EDGES.length(); ++k) {
		ivec2 edge = TESSELATION_EDGES[k];


		bool sign0 = (node.sign & (1u << edge[0])) != 0u;
		bool sign1 = (node.sign & (1u << edge[1])) != 0u;

		if(sign0 != sign1) {
			ivec4 order = TESSELATION_ORDERS[k];
            Vertex vertices[4];
            uint indices[4];

			for(int i = 0; i < 4 ; ++i) {
				int n = neighbors[order[i]];
                
                if(n >= 0 ) {
                    OctreeNodeCubeSerialized quadNode = nodes[n];
                    Vertex vertex;
                    vertex.position = quadNode.position;
                    vertex.normal = quadNode.normal;
                    vertex.texCoord = quadNode.texCoord;    
                    vertex.brushIndex = quadNode.brushIndex;
                    vertices[i] = vertex;
                    indices[i] = n;
                } else {
                    vertices[i].brushIndex = DISCARD_BRUSH_INDEX;
                    indices[i] = 0;
                }
			}
            handleTriangle(vertices[0], indices[0], vertices[2], indices[2], vertices[1], indices[1], sign1);
            handleTriangle(vertices[0], indices[0], vertices[3], indices[3], vertices[2], indices[2], sign1);
		}
    }    
}
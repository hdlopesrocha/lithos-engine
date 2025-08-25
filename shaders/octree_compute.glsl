#version 460


#define SpaceType_Empty 0
#define SpaceType_Surface 1
#define SpaceType_Solid 2
#define DISCARD_BRUSH_INDEX -1

#include<structs.glsl>

struct OctreeNodeCubeSerialized {
	float sdf[8];
    uint children[8];
    vec3 min;
    int brushIndex;
    vec3 length;
    uint bits;
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

vec3 getNormal(float sdf[8], float cubeLength) {
    float dx = cubeLength;
    float inv2dx = 1.0 / (2.0 * dx);

    float gx = (sdf[1] + sdf[5] + sdf[3] + sdf[7] - sdf[0] - sdf[4] - sdf[2] - sdf[6]) * 0.25;
    float gy = (sdf[2] + sdf[3] + sdf[6] + sdf[7] - sdf[0] - sdf[1] - sdf[4] - sdf[5]) * 0.25;
    float gz = (sdf[4] + sdf[5] + sdf[6] + sdf[7] - sdf[0] - sdf[1] - sdf[2] - sdf[3]) * 0.25;

    return normalize(vec3(gx, gy, gz) * inv2dx);
}

vec3 getNormalFromPosition(float sdf[8], vec3 cubeMin, vec3 cubeLength, vec3 position) {
    vec3 local = (position - cubeMin) / cubeLength; // [0,1]^3
    float x = local.x, y = local.y, z = local.z;

    float dx = (1-y)*(1-z)*(sdf[4]-sdf[0]) + (1-y)*z*(sdf[5]-sdf[1])
             + y*(1-z)*(sdf[6]-sdf[2]) + y*z*(sdf[7]-sdf[3]);

    float dy = (1-x)*(1-z)*(sdf[2]-sdf[0]) + (1-x)*z*(sdf[3]-sdf[1])
             + x*(1-z)*(sdf[6]-sdf[4]) + x*z*(sdf[7]-sdf[5]);

    float dz = (1-x)*(1-y)*(sdf[1]-sdf[0]) + (1-x)*y*(sdf[3]-sdf[2])
             + x*(1-y)*(sdf[5]-sdf[4]) + x*y*(sdf[7]-sdf[6]);

    dx /= cubeLength.x;
    dy /= cubeLength.y;
    dz /= cubeLength.z;

    return normalize(vec3(dx, dy, dz));
}

vec3 getAveragePosition(float sdf[8], vec3 min, vec3 length) {
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

vec3 interpolate(vec3 p0, vec3 p1, float v0, float v1) {
    float t = v0 / (v0 - v1);  // Linear interpolation weight
    return mix(p0, p1, t);     // Equivalent to: p0 + t * (p1 - p0)
}


vec3 solveQEF(mat3 ATA, vec3 ATb, vec3 fallback) {
    // Add tiny regularization for numerical safety
    ATA += mat3(1e-6);

    float det = determinant(ATA);
    if (abs(det) < 1e-12) {
        return fallback; // degenerate case
    }

    // GLSL's inverse() is okay here since it's just 3x3
    return inverse(ATA) * ATb;
}

// getShift(i): i is bit-coded as xyz (x=bit 2, y=bit 1, z=bit 0)
vec3 getGradientFromCorners(vec3 localPos, float sdfs[8]) {
    // central differences using your bit-based cube layout
    float dx = (sdfs[4] - sdfs[0] + sdfs[5] - sdfs[1] + sdfs[6] - sdfs[2] + sdfs[7] - sdfs[3]) * 0.25;
    float dy = (sdfs[2] - sdfs[0] + sdfs[3] - sdfs[1] + sdfs[6] - sdfs[4] + sdfs[7] - sdfs[5]) * 0.25;
    float dz = (sdfs[1] - sdfs[0] + sdfs[3] - sdfs[2] + sdfs[5] - sdfs[4] + sdfs[7] - sdfs[6]) * 0.25;

    return normalize(vec3(dx, dy, dz));
}

vec3 dualContouringQEF(vec3 min, vec3 length, float sdfs[8]) {

    mat3 ATA = mat3(0.0);
    vec3 ATb = vec3(0.0);
    int count = 0;

    vec3 cornerPos[8];
    for (int i = 0; i < 8; ++i)
        cornerPos[i] = min + vec3(getShift(i)) * length;

    for (int e = 0; e < 12; ++e) {
        int i0 = SDF_EDGES[e].x;
        int i1 = SDF_EDGES[e].y;

        float d0 = sdfs[i0];
        float d1 = sdfs[i1];

		bool sign0 = d0 < 0.0f;
		bool sign1 = d1 < 0.0f;
        if (sign0 != sign1) {
            vec3 p0 = cornerPos[i0];
            vec3 p1 = cornerPos[i1];
            vec3 pos = interpolate(p0, p1, d0, d1);
            //vec3 normal = getGradientFromCorners((pos-min)/length, sdfs);
            vec3 normal = getNormalFromPosition(sdfs, min, length, pos); // crude gradient along the edge

            ATA += outerProduct(normal, normal);       // AᵀA += n * nᵀ
            ATb += dot(normal, pos) * normal;        // Aᵀb += (n·p) * n

            count++;

            shaderOutput.result4f0 = vec4(count, length.length(), determinant(ATA), 0.0f);
        }
    }
    vec3 center = getAveragePosition(sdfs, min, length);

    if (count == 0) return center;

    vec3 result = solveQEF(ATA, ATb, center);
    return result;
}

vec3 getPosition(float sdf[8], vec3 min, vec3 length) {
    vec3 estimate = dualContouringQEF(min, length, sdf);
    if(!contains(min, length, estimate)) {
        estimate = getAveragePosition(sdf, min, length);
    }
   return clamp(estimate, min, min + length);
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

    for(int k = 0 ; k < tessEdge.length(); ++k) {
		ivec2 edge = tessEdge[k];

        float d0 = node.sdf[edge[0]];
        float d1 = node.sdf[edge[1]];

		bool sign0 = d0 < 0.0f;
		bool sign1 = d1 < 0.0f;

		if(sign0 != sign1) {
			ivec4 order = tessOrder[k];
            Vertex vertices[4];
            uint indices[4];

			for(int i = 0; i < 4 ; ++i) {
				int n = neighbors[order[i]];
                
                if(n >= 0 ) {
                    OctreeNodeCubeSerialized quadNode = nodes[n];

                    Vertex vertex;
                    vertex.brushIndex = quadNode.brushIndex;
                    vertex.position = vec4(getAveragePosition(quadNode.sdf, quadNode.min, quadNode.length), 0.0f);
                    vertex.normal = vec4(getNormalFromPosition(quadNode.sdf, quadNode.min, quadNode.length, vertex.position.xyz), 0.0f);
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
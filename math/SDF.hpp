#ifndef SDF_HPP
#define SDF_HPP

#include <glm/glm.hpp>
#include "math.hpp"

#define SDF_TYPE_SPHERE 0
#define SDF_TYPE_BOX 1
#define SDF_TYPE_CAPSULE 2
#define SDF_TYPE_HEIGHTMAP 3

enum SdfType {
    SPHERE, BOX, CAPSULE, HEIGHTMAP, OCTREE_DIFFERENCE};
const char* toString(SdfType t);


//      6-----7
//     /|    /|
//    4z+---5 |
//    | 2y--+-3
//    |/    |/
//    0-----1x
static const glm::ivec2 SDF_EDGES[12] = {
    {0, 1}, 
    {1, 3}, 
    {3, 2}, 
    {2, 0},
    {4, 5},
    {5, 7}, 
    {7, 6}, 
    {6, 4},
    {0, 4}, 
    {1, 5}, 
    {2, 6}, 
    {3, 7}
}; 

class SDF
{
public:
	SDF();
	~SDF();

    static float opUnion( float d1, float d2 );
    static float opSubtraction( float d1, float d2 );
    static float opIntersection( float d1, float d2 );
    static float opXor(float d1, float d2 );

    static float opSmoothUnion( float d1, float d2, float k );
    static float opSmoothSubtraction( float d1, float d2, float k );
    static float opSmoothIntersection( float d1, float d2, float k );

	static float box(glm::vec3 p, const glm::vec3 len);
	static float sphere(glm::vec3 p, const float r);
    static float capsule(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r );

    static glm::vec3 getPosition(float *sdf, const BoundingCube &cube);
    static glm::vec3 getNormal(float* sdf, const BoundingCube& cube);
    static glm::vec3 getNormalFromPosition(float* sdf, const BoundingCube& cube, const glm::vec3& position);
    static void getChildSDF(float * sdf, int i , float * result);
    static void copySDF(float * src, float * dst);
    static float interpolate(float * sdf, glm::vec3 position, BoundingCube cube);

    static SpaceType eval(float * sdf);
};



class SignedDistanceFunction {
    public:
    virtual SdfType getType() const = 0; 
	virtual float distance(const glm::vec3 p) const = 0;
};

class SphereDistanceFunction : public SignedDistanceFunction {
    public:
    glm::vec3 position;
    float radius;	

	SphereDistanceFunction(glm::vec3 pos, float radius);
	float distance(const glm::vec3 p) const override;
    SdfType getType() const override; 
};

class BoxDistanceFunction : public SignedDistanceFunction {
    public:
    glm::vec3 position;
    glm::vec3 length;

	BoxDistanceFunction(glm::vec3 pos, glm::vec3 len);
	float distance(const glm::vec3 p) const override;
    SdfType getType() const override; 
};

class CapsuleDistanceFunction : public SignedDistanceFunction {
    public:	
    glm::vec3 a;
    glm::vec3 b;
    float radius;

    CapsuleDistanceFunction(glm::vec3 a, glm::vec3 b, float r);
	float distance(const glm::vec3 p) const override;
    SdfType getType() const override; 
};

class HeightMapDistanceFunction : public SignedDistanceFunction {
	const HeightMap &map;
	public:
	HeightMapDistanceFunction(const HeightMap &map);
	float distance(const glm::vec3 p) const override;
    SdfType getType() const override; 
};

#endif
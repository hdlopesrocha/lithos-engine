#ifndef SDF_HPP
#define SDF_HPP

#include <glm/glm.hpp>
#include "math.hpp"
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
	virtual float distance(const glm::vec3 p) const = 0;
};

class SphereDistanceFunction : public SignedDistanceFunction {
	BoundingSphere sphere;
	public:	
	SphereDistanceFunction(BoundingSphere sphere);
	float distance(const glm::vec3 p) const override;
};

class BoxDistanceFunction : public SignedDistanceFunction {
	BoundingBox box;
	public:
	BoxDistanceFunction(BoundingBox box);
	float distance(const glm::vec3 p) const override;
};

class HeightMapDistanceFunction : public SignedDistanceFunction {
	const HeightMap &map;
	public:
	HeightMapDistanceFunction(const HeightMap &map);
	float distance(const glm::vec3 p) const override;
};

#endif
#ifndef SDF_HPP
#define SDF_HPP

#include <glm/glm.hpp>
#include "math.hpp"

#define SDF_TYPE_SPHERE 0
#define SDF_TYPE_BOX 1
#define SDF_TYPE_CAPSULE 2
#define SDF_TYPE_HEIGHTMAP 3

enum SdfType {
    SPHERE, BOX, CAPSULE, HEIGHTMAP, OCTREE_DIFFERENCE, OCTAHEDRON};
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
    static float octahedron(glm::vec3 p, float s);

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
    virtual glm::vec3 getCenter() const = 0;
};


class SphereDistanceFunction : public SignedDistanceFunction {
    public:
    glm::vec3 center;
    float radius;	

	SphereDistanceFunction(glm::vec3 pos, float radius);
	float distance(const glm::vec3 p) const override;
    SdfType getType() const override; 
    glm::vec3 getCenter() const override;

};

class BoxDistanceFunction : public SignedDistanceFunction {
    public:
    glm::vec3 center;
    glm::vec3 length;

	BoxDistanceFunction(glm::vec3 pos, glm::vec3 len);
	float distance(const glm::vec3 p) const override;
    SdfType getType() const override; 
    glm::vec3 getCenter() const override;

};

class CapsuleDistanceFunction : public SignedDistanceFunction {
    public:	
    glm::vec3 a;
    glm::vec3 b;
    float radius;

    CapsuleDistanceFunction(glm::vec3 a, glm::vec3 b, float r);
	float distance(const glm::vec3 p) const override;
    SdfType getType() const override; 
    glm::vec3 getCenter() const override;

};

class HeightMapDistanceFunction : public SignedDistanceFunction {
	public:
	const HeightMap &map;
	HeightMapDistanceFunction(const HeightMap &map);
	float distance(const glm::vec3 p) const override;
    SdfType getType() const override; 
    glm::vec3 getCenter() const override;

};

class OctahedronDistanceFunction : public SignedDistanceFunction {
    public:
    glm::vec3 center;
    float radius;	

	OctahedronDistanceFunction(glm::vec3 pos, float radius);
	float distance(const glm::vec3 p) const override;
    SdfType getType() const override; 
    glm::vec3 getCenter() const override;

};

class WrappedSignedDistanceFunction : public SignedDistanceFunction {
    protected:
    SignedDistanceFunction * function;
    float bias;
    public:
    WrappedSignedDistanceFunction(SignedDistanceFunction * function, float bias) : function(function), bias(bias) {

    }

    virtual ContainmentType check(const BoundingCube &cube) const = 0;
    virtual bool isContained(const BoundingCube &cube) const = 0;

    SdfType getType() const override {
        return function->getType();
    }    

	float distance(const glm::vec3 p) const override {
        return function->distance(p);
    }

    glm::vec3 getCenter() const override {
        return function->getCenter();
    };

};

class WrappedSphere : public WrappedSignedDistanceFunction {
    public:
    WrappedSphere(SphereDistanceFunction * function, float bias) : WrappedSignedDistanceFunction(function, bias) {

    }

    BoundingSphere getSphere() const {
        SphereDistanceFunction * f = (SphereDistanceFunction*) function;
        return BoundingSphere(f->center, f->radius + bias);
    };

    ContainmentType check(const BoundingCube &cube) const override {
        BoundingSphere sphere = getSphere();
        return sphere.test(cube);
    };

    bool isContained(const BoundingCube &cube) const override {
        BoundingSphere sphere = getSphere();
        return cube.contains(sphere);
    };

};

class WrappedBox : public WrappedSignedDistanceFunction {
    public:
    WrappedBox(BoxDistanceFunction * function, float bias) : WrappedSignedDistanceFunction(function, bias) {

    }

    BoundingSphere getSphere() const {
        BoxDistanceFunction * f = (BoxDistanceFunction*) function;
        return BoundingSphere(f->center, glm::length(f->length)+ bias);
    };

    ContainmentType check(const BoundingCube &cube) const override {
        BoundingSphere sphere = getSphere();
        return sphere.test(cube);
    };

    bool isContained(const BoundingCube &cube) const override {
        BoundingSphere sphere = getSphere();
        return cube.contains(sphere);
    };

};

class WrappedCapsule : public WrappedSignedDistanceFunction {
    public:
    WrappedCapsule(CapsuleDistanceFunction * function, float bias) : WrappedSignedDistanceFunction(function, bias) {

    }

    BoundingSphere getSphere() const {
        CapsuleDistanceFunction * f = (CapsuleDistanceFunction*) function;
        return BoundingSphere(0.5f*(f->a + f->b), glm::distance(f->a, f->b) + f->radius + bias);
    };

    ContainmentType check(const BoundingCube &cube) const override {
        BoundingSphere sphere = getSphere();
        return sphere.test(cube);
    };

    bool isContained(const BoundingCube &cube) const override {
        BoundingSphere sphere = getSphere();
        return cube.contains(sphere);
    };

};

class WrappedHeightMap : public WrappedSignedDistanceFunction {
    public:
    WrappedHeightMap(HeightMapDistanceFunction * function, float bias) : WrappedSignedDistanceFunction(function, bias) {

    }

    BoundingBox getBox() const {
        HeightMapDistanceFunction * f = (HeightMapDistanceFunction*) function;
        return BoundingBox(f->map.getMin()-glm::vec3(bias), f->map.getMax()+glm::vec3(bias));
    }
        
    ContainmentType check(const BoundingCube &cube) const override {
        BoundingBox box = getBox();
        return box.test(cube);
    };

    bool isContained(const BoundingCube &cube) const override {
        BoundingBox box = getBox();
        return cube.contains(box);
    };
};

class WrappedOctahedron : public WrappedSignedDistanceFunction {
    public:
    WrappedOctahedron(OctahedronDistanceFunction * function, float bias) : WrappedSignedDistanceFunction(function, bias) {

    }

    BoundingSphere getSphere() const {
        OctahedronDistanceFunction * f = (OctahedronDistanceFunction*) function;
        return BoundingSphere(f->center, f->radius + bias);
    };

    ContainmentType check(const BoundingCube &cube) const override {
        BoundingSphere sphere = getSphere();
        return sphere.test(cube);
    };

    bool isContained(const BoundingCube &cube) const override {
        BoundingSphere sphere = getSphere();
        return cube.contains(sphere);
    };

};


#endif
#ifndef SDF_HPP
#define SDF_HPP

#include <glm/glm.hpp>
#include "math.hpp"

#define SDF_TYPE_SPHERE 0
#define SDF_TYPE_BOX 1
#define SDF_TYPE_CAPSULE 2
#define SDF_TYPE_HEIGHTMAP 3

enum SdfType {
    SPHERE, BOX, CAPSULE, HEIGHTMAP, OCTREE_DIFFERENCE, OCTAHEDRON, PYRAMID};
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

class Transformation {
	public:
	glm::vec3 scale;
	glm::vec3 translate;
    glm::quat quaternion;

    Transformation() {
        this->scale = glm::vec3(1.0f,1.0f,1.0f);
        this->translate = glm::vec3(0.0f,0.0f,0.0f);
        this->quaternion = getRotation(0.0f, 0.0f, 0.0f);
    }


	Transformation(glm::vec3 scale, glm::vec3 translate, float yaw, float pitch, float roll) {
		this->scale = scale;
		this->translate = translate;
		this->quaternion = getRotation(yaw, pitch, roll);
	}


	glm::quat getRotation(float yaw, float pitch, float roll) {
		return 	glm::angleAxis(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f)) * // Yaw (Y-axis)
				glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f)) * // Pitch (X-axis)
				glm::angleAxis(glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));  // Roll (Z-axis)
	}


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
    static float pyramid(glm::vec3 p, float h );

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
	virtual float distance(const glm::vec3 p, Transformation model) const = 0;
    virtual glm::vec3 getCenter(Transformation model) const = 0;
};


class SphereDistanceFunction : public SignedDistanceFunction {
    public:
    glm::vec3 center;
    float radius;	

	SphereDistanceFunction(glm::vec3 pos, float radius);
	float distance(const glm::vec3 p, Transformation model) const override;
    SdfType getType() const override; 
    glm::vec3 getCenter(Transformation model) const override;

};

class BoxDistanceFunction : public SignedDistanceFunction {
    public:
    glm::vec3 center;
    glm::vec3 length;

	BoxDistanceFunction(glm::vec3 center, glm::vec3 length);
	float distance(const glm::vec3 p, Transformation model) const override;
    SdfType getType() const override; 
    glm::vec3 getCenter(Transformation model) const override;

};

class CapsuleDistanceFunction : public SignedDistanceFunction {
    public:	
    glm::vec3 a;
    glm::vec3 b;
    float radius;

    CapsuleDistanceFunction(glm::vec3 a, glm::vec3 b, float r);
	float distance(const glm::vec3 p, Transformation model) const override;
    SdfType getType() const override; 
    glm::vec3 getCenter(Transformation model) const override;

};

class HeightMapDistanceFunction : public SignedDistanceFunction {
	public:
	const HeightMap &map;
	HeightMapDistanceFunction(const HeightMap &map);
	float distance(const glm::vec3 p, Transformation model) const override;
    SdfType getType() const override; 
    glm::vec3 getCenter(Transformation model) const override;

};

class OctahedronDistanceFunction : public SignedDistanceFunction {
    public:
    glm::vec3 center;
    float radius;	

	OctahedronDistanceFunction(glm::vec3 center, float radius);
	float distance(const glm::vec3 p, Transformation model) const override;
    SdfType getType() const override; 
    glm::vec3 getCenter(Transformation model) const override;

};

class PyramidDistanceFunction : public SignedDistanceFunction {
    public:
    glm::vec3 base; 
    float height;

	PyramidDistanceFunction(glm::vec3 base, float height);
	float distance(const glm::vec3 p, Transformation model) const override;
    SdfType getType() const override; 
    glm::vec3 getCenter(Transformation model) const override;

};


class WrappedSignedDistanceFunction : public SignedDistanceFunction {
    protected:
    SignedDistanceFunction * function;
    float bias;
    Transformation &model;
    public:
    WrappedSignedDistanceFunction(SignedDistanceFunction * function, float bias, Transformation &model) : function(function), bias(bias), model(model) {

    }

    virtual ContainmentType check(const BoundingCube &cube) const = 0;
    virtual bool isContained(const BoundingCube &cube) const = 0;

    SdfType getType() const override {
        return function->getType();
    }    

	float distance(const glm::vec3 p, Transformation model) const override {
        return function->distance(p, model);
    }

    glm::vec3 getCenter(Transformation model) const override {
        return function->getCenter(model);
    };

};

class WrappedSphere : public WrappedSignedDistanceFunction {
    public:
    WrappedSphere(SphereDistanceFunction * function, float bias, Transformation &model) : WrappedSignedDistanceFunction(function, bias, model) {

    }

    BoundingSphere getSphere() const {
        SphereDistanceFunction * f = (SphereDistanceFunction*) function;
        return BoundingSphere(f->getCenter(model), f->radius*glm::length(model.scale) + bias);
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
    WrappedBox(BoxDistanceFunction * function, float bias, Transformation &model) : WrappedSignedDistanceFunction(function, bias, model) {

    }

    BoundingSphere getSphere() const {
        BoxDistanceFunction * f = (BoxDistanceFunction*) function;
        return BoundingSphere(f->getCenter(model), glm::length(f->length*model.scale)+ bias);
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
    WrappedCapsule(CapsuleDistanceFunction * function, float bias, Transformation &model) : WrappedSignedDistanceFunction(function, bias, model) {

    }

    BoundingSphere getSphere() const {
        CapsuleDistanceFunction * f = (CapsuleDistanceFunction*) function;
        return BoundingSphere(f->getCenter(model), (2.0f*f->radius+glm::distance(f->a, f->b))*glm::length(model.scale) + bias);
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
    WrappedHeightMap(HeightMapDistanceFunction * function, float bias, Transformation &model) : WrappedSignedDistanceFunction(function, bias, model) {

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
    WrappedOctahedron(OctahedronDistanceFunction * function, float bias, Transformation &model) : WrappedSignedDistanceFunction(function, bias, model) {

    }

    BoundingSphere getSphere() const {
        OctahedronDistanceFunction * f = (OctahedronDistanceFunction*) function;
        return BoundingSphere(f->getCenter(model), f->radius*glm::length(model.scale) + bias);
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

class WrappedPyramid : public WrappedSignedDistanceFunction {
    public:
    WrappedPyramid(PyramidDistanceFunction * function, float bias, Transformation &model) : WrappedSignedDistanceFunction(function, bias, model) {

    }

    float boundingSphereRadius(float width, float depth, float height) const {
        return glm::length(glm::vec3(width, height, depth));
    }

    BoundingSphere getSphere() const {
        PyramidDistanceFunction * f = (PyramidDistanceFunction*) function;
        return BoundingSphere(f->getCenter(model), f->height*glm::length(model.scale) + bias);
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
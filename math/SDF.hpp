#ifndef SDF_HPP
#define SDF_HPP

#include <glm/glm.hpp>
#include "math.hpp"

#define SDF_TYPE_SPHERE 0
#define SDF_TYPE_BOX 1
#define SDF_TYPE_CAPSULE 2
#define SDF_TYPE_HEIGHTMAP 3

enum SdfType {
    SPHERE, BOX, CAPSULE, HEIGHTMAP, OCTREE_DIFFERENCE, OCTAHEDRON, PYRAMID, TORUS
};
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
    static float torus(glm::vec3 p, glm::vec2 t );
    static float capsule(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r );
    static float octahedron(glm::vec3 p, float s);
    static float pyramid(const glm::vec3 &p, float h, float a);

    static glm::vec3 getPosition(float *sdf, const BoundingCube &cube);
    static glm::vec3 getAveragePosition(float *sdf, const BoundingCube &cube);
    static glm::vec3 getAveragePosition2(float *sdf, const BoundingCube &cube);
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
	virtual float distance(const glm::vec3 p, const Transformation &model) = 0;
    virtual glm::vec3 getCenter(const Transformation &model) const = 0;
};


class SphereDistanceFunction : public SignedDistanceFunction {
    public:
	
	SphereDistanceFunction();
	float distance(const glm::vec3 p, const Transformation &model) override;
    SdfType getType() const override; 
    glm::vec3 getCenter(const Transformation &model) const override;

};

class TorusDistanceFunction : public SignedDistanceFunction {
    public:
    glm::vec2 radius;	

	TorusDistanceFunction(glm::vec2 radius);
	float distance(const glm::vec3 p, const Transformation &model) override;
    SdfType getType() const override; 
    glm::vec3 getCenter(const Transformation &model) const override;

};

class BoxDistanceFunction : public SignedDistanceFunction {
    public:

	BoxDistanceFunction();
	float distance(const glm::vec3 p, const Transformation &model) override;
    SdfType getType() const override; 
    glm::vec3 getCenter(const Transformation &model) const override;

};

class CapsuleDistanceFunction : public SignedDistanceFunction {
    public:	
    glm::vec3 a;
    glm::vec3 b;
    float radius;

    CapsuleDistanceFunction(glm::vec3 a, glm::vec3 b, float r);
	float distance(const glm::vec3 p, const Transformation &model) override;
    SdfType getType() const override; 
    glm::vec3 getCenter(const Transformation &model) const override;

};

class HeightMapDistanceFunction : public SignedDistanceFunction {
	public:
	HeightMap * map;
	HeightMapDistanceFunction(HeightMap * map);
	float distance(const glm::vec3 p, const Transformation &model) override;
    SdfType getType() const override; 
    glm::vec3 getCenter(const Transformation &model) const override;

};

class OctahedronDistanceFunction : public SignedDistanceFunction {
    public:
 
	OctahedronDistanceFunction();
	float distance(const glm::vec3 p, const Transformation &model) override;
    SdfType getType() const override; 
    glm::vec3 getCenter(const Transformation &model) const override;

};

class PyramidDistanceFunction : public SignedDistanceFunction {
    public:

	PyramidDistanceFunction();
	float distance(const glm::vec3 p, const Transformation &model) override;
    SdfType getType() const override; 
    glm::vec3 getCenter(const Transformation &model) const override;

};



class WrappedSignedDistanceFunction : public SignedDistanceFunction {
    protected:
    SignedDistanceFunction * function;
    std::unordered_map<glm::vec3, float> cacheSDF;
    std::mutex mtx;
    public:
    bool cacheEnabled = false;

    WrappedSignedDistanceFunction(SignedDistanceFunction * function) : function(function) {

    }
    virtual ~WrappedSignedDistanceFunction() = default;


    virtual ContainmentType check(const BoundingCube &cube, const Transformation &model, float bias) const = 0;
    virtual float getLength(const Transformation &model, float bias) const = 0;

    virtual bool isContained(const BoundingCube &cube, const Transformation &model, float bias) const = 0;

    SdfType getType() const override {
        return function->getType();
    }    

	float distance(const glm::vec3 p, const Transformation &model) override {
        if(cacheEnabled){
            mtx.lock();
            auto it = cacheSDF.find(p);
            if (it != cacheSDF.end()) {
                float value = it->second;
                mtx.unlock();
                return value;
            }
            mtx.unlock();
            float result = function->distance(p, model);
            

            mtx.lock();
            cacheSDF[p] = result;
            mtx.unlock();


            return result;
        } else {
           return function->distance(p, model);
        }
    }

    glm::vec3 getCenter(const Transformation &model) const override {
        return function->getCenter(model);
    };

};

class WrappedSphere : public WrappedSignedDistanceFunction {
    public:
    WrappedSphere(SphereDistanceFunction * function) : WrappedSignedDistanceFunction(function) {

    }

    BoundingSphere getSphere(const Transformation &model, float bias) const {
        SphereDistanceFunction * f = (SphereDistanceFunction*) function;
        return BoundingSphere(f->getCenter(model), glm::length(model.scale) + bias);
    };

    ContainmentType check(const BoundingCube &cube, const Transformation &model, float bias) const override {
        BoundingSphere sphere = getSphere(model, bias);
        return sphere.test(cube);
    };

    bool isContained(const BoundingCube &cube, const Transformation &model, float bias) const override {
        BoundingSphere sphere = getSphere(model, bias);
        return cube.contains(sphere);
    };

    float getLength(const Transformation &model, float bias) const override {
        return glm::length(model.scale) + bias;
    };

};

class WrappedTorus : public WrappedSignedDistanceFunction {
    public:
    WrappedTorus(TorusDistanceFunction * function) : WrappedSignedDistanceFunction(function) {

    }

    BoundingSphere getSphere(const Transformation &model, float bias) const {
        TorusDistanceFunction * f = (TorusDistanceFunction*) function;
        return BoundingSphere(f->getCenter(model), glm::length(model.scale) + bias);
    };

    ContainmentType check(const BoundingCube &cube, const Transformation &model, float bias) const override {
        BoundingSphere sphere = getSphere(model, bias);
        return sphere.test(cube);
    };

    bool isContained(const BoundingCube &cube, const Transformation &model, float bias) const override {
        BoundingSphere sphere = getSphere(model, bias);
        return cube.contains(sphere);
    };

    float getLength(const Transformation &model, float bias) const override {
        TorusDistanceFunction * f = (TorusDistanceFunction*) function;
        float R = f->radius.x;
        float r = f->radius.y;

        float Rx = (R + r) * model.scale.x;
        float Ry = r * model.scale.y;
        float Rz = (R + r) * model.scale.z;

        return glm::max(glm::max(Rx, Ry), Rz) + bias;
    };

};


class WrappedBox : public WrappedSignedDistanceFunction {
    public:
    WrappedBox(BoxDistanceFunction * function) : WrappedSignedDistanceFunction(function) {

    }

    BoundingSphere getSphere(const Transformation &model, float bias) const {
        BoxDistanceFunction * f = (BoxDistanceFunction*) function;
        return BoundingSphere(f->getCenter(model), glm::length(model.scale)+ bias);
    };

    ContainmentType check(const BoundingCube &cube, const Transformation &model, float bias) const override {
        BoundingSphere sphere = getSphere(model, bias);
        return sphere.test(cube);
    };

    bool isContained(const BoundingCube &cube, const Transformation &model, float bias) const override {
        BoundingSphere sphere = getSphere(model, bias);
        return cube.contains(sphere);
    };

    float getLength(const Transformation &model, float bias) const override {
        return glm::length(model.scale) + bias;
    };
};

class WrappedCapsule : public WrappedSignedDistanceFunction {
    public:
    WrappedCapsule(CapsuleDistanceFunction * function) : WrappedSignedDistanceFunction(function) {

    }

    BoundingBox getBox(const Transformation &model, float bias) const {
        CapsuleDistanceFunction * f = (CapsuleDistanceFunction*) function;
        glm::vec3 min = glm::min(f->a, f->b)*glm::length(model.scale)+model.translate;
        glm::vec3 max = glm::max(f->a, f->b)*glm::length(model.scale)+model.translate;
        glm::vec3 len = glm::vec3(f->radius  + bias);


        return BoundingBox(min - len, max + len);
    };

    ContainmentType check(const BoundingCube &cube, const Transformation &model, float bias) const override {
        BoundingBox box = getBox(model, bias);
        return box.test(cube);
    };

    bool isContained(const BoundingCube &cube, const Transformation &model, float bias) const override {
        BoundingBox box = getBox(model, bias);
        return cube.contains(box);
    };
    float getLength(const Transformation &model, float bias) const override {
        BoundingBox box = getBox(model, bias);
        return glm::distance(box.getMin(), box.getMax());
    };
};

class WrappedHeightMap : public WrappedSignedDistanceFunction {
    public:
    WrappedHeightMap(HeightMapDistanceFunction * function) : WrappedSignedDistanceFunction(function) {

    }

    BoundingBox getBox(float bias) const {
        HeightMapDistanceFunction * f = (HeightMapDistanceFunction*) function;
        return BoundingBox(f->map->getMin()-glm::vec3(bias), f->map->getMax()+glm::vec3(bias));
    }
        
    ContainmentType check(const BoundingCube &cube, const Transformation &model, float bias) const override {
        BoundingBox box = getBox(bias);
        return box.test(cube);
    };

    bool isContained(const BoundingCube &cube, const Transformation &model, float bias) const override {
        BoundingBox box = getBox(bias);
        return cube.contains(box);
    };
    
    float getLength(const Transformation &model, float bias) const override {
        HeightMapDistanceFunction * f = (HeightMapDistanceFunction*) function;
        return glm::distance(f->map->getMin(), f->map->getMax()) + bias;
    };
};

class WrappedOctahedron : public WrappedSignedDistanceFunction {
    public:
    WrappedOctahedron(OctahedronDistanceFunction * function) : WrappedSignedDistanceFunction(function) {

    }

    BoundingSphere getSphere(const Transformation &model, float bias) const {
        OctahedronDistanceFunction * f = (OctahedronDistanceFunction*) function;
        return BoundingSphere(f->getCenter(model), glm::length(model.scale) + bias);
    };

    ContainmentType check(const BoundingCube &cube, const Transformation &model, float bias) const override {
        BoundingSphere sphere = getSphere(model, bias);
        return sphere.test(cube);
    };

    bool isContained(const BoundingCube &cube, const Transformation &model, float bias) const override {
        BoundingSphere sphere = getSphere(model, bias);
        return cube.contains(sphere);
    };

    float getLength(const Transformation &model, float bias) const override {
        return glm::length(model.scale) + bias;
    };

};

class WrappedPyramid : public WrappedSignedDistanceFunction {
    public:
    WrappedPyramid(PyramidDistanceFunction * function) : WrappedSignedDistanceFunction(function) {

    }

    float boundingSphereRadius(float width, float depth, float height) const {
        return glm::length(glm::vec3(width, height, depth));
    }

    BoundingSphere getSphere(const Transformation &model, float bias) const {
        PyramidDistanceFunction * f = (PyramidDistanceFunction*) function;
        return BoundingSphere(f->getCenter(model), 0.5f * glm::length(model.scale) + bias);
    };

    ContainmentType check(const BoundingCube &cube, const Transformation &model, float bias) const override {
        BoundingSphere sphere = getSphere(model, bias);
        return sphere.test(cube);
    };

    bool isContained(const BoundingCube &cube, const Transformation &model, float bias) const override {
        BoundingSphere sphere = getSphere(model, bias);
        return cube.contains(sphere);
    };
    float getLength(const Transformation &model, float bias) const override {
        return glm::length(model.scale) + bias;
    };
};

#endif
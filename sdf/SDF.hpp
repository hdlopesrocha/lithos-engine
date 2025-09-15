#ifndef SDF_HPP
#define SDF_HPP

#include <glm/glm.hpp>
#include "../math/math.hpp"

enum SdfType {
    SPHERE, 
    BOX, 
    CAPSULE, 
    HEIGHTMAP, 
    OCTREE_DIFFERENCE, 
    OCTAHEDRON, 
    PYRAMID, 
    TORUS, 
    CONE
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
    static float cone(glm::vec3 p);

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

    virtual void accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const = 0;
    virtual ContainmentType check(const BoundingCube &cube, const Transformation &model, float bias) const = 0;
    virtual float getLength(const Transformation &model, float bias) const = 0;
    virtual bool isContained(const BoundingCube &cube, const Transformation &model, float bias) const = 0;
    virtual const char* getLabel() const = 0;

    SdfType getType() const override {
        return function->getType();
    }    

    SignedDistanceFunction * getFunction() {
        return function;
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

class SphereDistanceFunction : public SignedDistanceFunction {
    public:
	
	SphereDistanceFunction();
	float distance(const glm::vec3 p, const Transformation &model) override;
    SdfType getType() const override; 
    glm::vec3 getCenter(const Transformation &model) const override;
};

class WrappedSphere : public WrappedSignedDistanceFunction {
    public:
    WrappedSphere(SphereDistanceFunction * function);
    ~WrappedSphere();
    BoundingSphere getSphere(const Transformation &model, float bias) const;
    ContainmentType check(const BoundingCube &cube, const Transformation &model, float bias) const override;
    bool isContained(const BoundingCube &cube, const Transformation &model, float bias) const override;
    float getLength(const Transformation &model, float bias) const override;
    void accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const override;
    const char* getLabel() const override;
};

class TorusDistanceFunction : public SignedDistanceFunction {
    public:
    glm::vec2 radius;	

	TorusDistanceFunction(glm::vec2 radius);
	float distance(const glm::vec3 p, const Transformation &model) override;
    SdfType getType() const override; 
    glm::vec3 getCenter(const Transformation &model) const override;

};

class WrappedTorus : public WrappedSignedDistanceFunction {
    public:
    WrappedTorus(TorusDistanceFunction * function);
    ~WrappedTorus();
    BoundingSphere getSphere(const Transformation &model, float bias) const;
    ContainmentType check(const BoundingCube &cube, const Transformation &model, float bias) const override;
    bool isContained(const BoundingCube &cube, const Transformation &model, float bias) const override;
    float getLength(const Transformation &model, float bias) const override;
    void accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const override;
    const char* getLabel() const override;
};

class BoxDistanceFunction : public SignedDistanceFunction {
    public:

	BoxDistanceFunction();
	float distance(const glm::vec3 p, const Transformation &model) override;
    SdfType getType() const override; 
    glm::vec3 getCenter(const Transformation &model) const override;

};

class WrappedBox : public WrappedSignedDistanceFunction {
    public:
    WrappedBox(BoxDistanceFunction * function);
    ~WrappedBox();
    BoundingSphere getSphere(const Transformation &model, float bias) const;
    ContainmentType check(const BoundingCube &cube, const Transformation &model, float bias) const override;
    bool isContained(const BoundingCube &cube, const Transformation &model, float bias) const override;
    float getLength(const Transformation &model, float bias) const override;
    void accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const override;
    const char* getLabel() const override;
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

class WrappedCapsule : public WrappedSignedDistanceFunction {
    public:
    WrappedCapsule(CapsuleDistanceFunction * function);
    ~WrappedCapsule();
    BoundingBox getBox(const Transformation &model, float bias) const;
    ContainmentType check(const BoundingCube &cube, const Transformation &model, float bias) const override;
    bool isContained(const BoundingCube &cube, const Transformation &model, float bias) const override;
    float getLength(const Transformation &model, float bias) const override;
    void accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const override;
    const char* getLabel() const override;
};

class HeightMapDistanceFunction : public SignedDistanceFunction {
	public:
	HeightMap * map;
	HeightMapDistanceFunction(HeightMap * map);
	float distance(const glm::vec3 p, const Transformation &model) override;
    SdfType getType() const override; 
    glm::vec3 getCenter(const Transformation &model) const override;

};

class WrappedHeightMap : public WrappedSignedDistanceFunction {
    public:
    WrappedHeightMap(HeightMapDistanceFunction * function);
    ~WrappedHeightMap();
    BoundingBox getBox(float bias) const;
    ContainmentType check(const BoundingCube &cube, const Transformation &model, float bias) const override;
    bool isContained(const BoundingCube &cube, const Transformation &model, float bias) const override;
    float getLength(const Transformation &model, float bias) const override;
    void accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const override;
    const char* getLabel() const override;
};

class OctahedronDistanceFunction : public SignedDistanceFunction {
    public:
 
	OctahedronDistanceFunction();
	float distance(const glm::vec3 p, const Transformation &model) override;
    SdfType getType() const override; 
    glm::vec3 getCenter(const Transformation &model) const override;

};

class WrappedOctahedron : public WrappedSignedDistanceFunction {
    public:
    WrappedOctahedron(OctahedronDistanceFunction * function);
    ~WrappedOctahedron();
    BoundingSphere getSphere(const Transformation &model, float bias) const;
    ContainmentType check(const BoundingCube &cube, const Transformation &model, float bias) const override;
    bool isContained(const BoundingCube &cube, const Transformation &model, float bias) const override;
    float getLength(const Transformation &model, float bias) const override;
    void accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const override;
    const char* getLabel() const override;
};

class PyramidDistanceFunction : public SignedDistanceFunction {
    public:

	PyramidDistanceFunction();
	float distance(const glm::vec3 p, const Transformation &model) override;
    SdfType getType() const override; 
    glm::vec3 getCenter(const Transformation &model) const override;

};

class WrappedPyramid : public WrappedSignedDistanceFunction {
    public:
    WrappedPyramid(PyramidDistanceFunction * function);
    ~WrappedPyramid();
    float boundingSphereRadius(float width, float depth, float height) const;
    BoundingSphere getSphere(const Transformation &model, float bias) const;
    ContainmentType check(const BoundingCube &cube, const Transformation &model, float bias) const override;
    bool isContained(const BoundingCube &cube, const Transformation &model, float bias) const override;
    float getLength(const Transformation &model, float bias) const override;
    void accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const override;
    const char* getLabel() const override;
};

class ConeDistanceFunction : public SignedDistanceFunction {
    public:
	
	ConeDistanceFunction();
	float distance(const glm::vec3 p, const Transformation &model) override;
    SdfType getType() const override; 
    glm::vec3 getCenter(const Transformation &model) const override;
};

class WrappedCone : public WrappedSignedDistanceFunction {
    public:
    WrappedCone(ConeDistanceFunction * function);
    ~WrappedCone();
    BoundingSphere getSphere(const Transformation &model, float bias) const;
    ContainmentType check(const BoundingCube &cube, const Transformation &model, float bias) const override;
    bool isContained(const BoundingCube &cube, const Transformation &model, float bias) const override;
    float getLength(const Transformation &model, float bias) const override;
    void accept(BoundingVolumeVisitor &visitor, const Transformation &model, float bias) const override;
    const char* getLabel() const override;
};


#endif
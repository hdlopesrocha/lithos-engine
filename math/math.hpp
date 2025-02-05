#ifndef MATH_HPP
#define MATH_HPP

#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <map>


class BoundingSphere;
class BoundingBox;

enum ContainmentType {
	Contains,
	Intersects,
	Disjoint
};

struct Plane {
	glm::vec3 normal;
	float d;
	public:
	Plane(glm::vec3 normal, glm::vec3 point);
	float distance(glm::vec3 point);
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    int texIndex;

    Vertex(){
		this->position = glm::vec3(0);
    	this->normal = glm::vec3(0);
    	this->texCoord = glm::vec2(0);
    	this->texIndex = 0;
    }

    Vertex(glm::vec3 pos) : Vertex(){
    	this->position = pos;
    }

    Vertex(glm::vec3 pos,glm::vec3 normal,glm::vec2 texCoord, uint texIndex): Vertex() {
    	this->position = pos;
    	this->normal = normal;
    	this->texCoord = texCoord;
    	this->texIndex = texIndex;
    }


    std::string toKey() {
		std::stringstream ss;
		ss << "[" << position.x <<  " " <<  position.y <<  " " <<  position.z << "]";
		return ss.str();
    }

    std::string toString() {
		std::stringstream ss;
		ss << "{"
			<< "\"p\":" << "[" << position.x <<  "," <<  position.y <<  "," <<  position.z << "],"
			<< "\"n\":" << "[" << normal.x <<  "," <<  normal.y <<  "," <<  normal.z << "],"
			<< "\"c\":" << "[" << texCoord.x <<  "," <<  texCoord.y << "],"
			<< "\"i\":" << texIndex
		<< "}";
		return ss.str();
    }
};

struct Triangle {
	public:
		Vertex v[3];

		Triangle(Vertex v1, Vertex v2, Vertex v3){
			this->v[0] = v1;
			this->v[1] = v2;
			this->v[2] = v3;
		}

		Triangle flip(){
			Vertex t = this->v[1];
			this->v[1]= this->v[2];
			this->v[2]= t;
			return *this;
		}
};

enum SpaceType {
    Empty,
    Surface,
    Solid
};


class BoundingCube {
	private: 
		glm::vec3 min;
		float length;

	public: 
		BoundingCube(glm::vec3 min, float length);
		float getMaxX();
		float getMaxY();
		float getMaxZ();
		float getMinX();
		float getMinY();
		float getMinZ();
		glm::vec3 getMin();
		glm::vec3 getMax();
		glm::vec3 getCenter();
		float getLength();
		void setLength(float l);
		void setMin(glm::vec3 v);
		bool contains(glm::vec3 point);
		bool contains(BoundingSphere sphere);
		bool contains(BoundingCube cube);
		bool contains(BoundingBox box);

};


class BoundingSphere {
	public: 
		glm::vec3 center;
		float radius;
		BoundingSphere();		
		BoundingSphere(glm::vec3 center, float radius);
		bool contains(glm::vec3 point);
		ContainmentType test(BoundingCube cube);
		bool intersects(BoundingCube cube);
};


class BoundingBox {
	private: 
		glm::vec3 min;
		glm::vec3 max;

	public: 
		BoundingBox(glm::vec3 min, glm::vec3 max);
		BoundingBox();
		float getMaxX();
		float getMaxY();
		float getMaxZ();
		float getMinX();
		float getMinY();
		float getMinZ();
		glm::vec3 getMin();
		glm::vec3 getMax();
		glm::vec3 getCenter();
		glm::vec3 getLength();
		void setMin(glm::vec3 v);
		void setMax(glm::vec3 v);
		ContainmentType test(BoundingCube cube);
		bool intersects(BoundingSphere sphere);
		bool contains(glm::vec3 point);
};


class HeightFunction {
	public:
		virtual float getHeightAt(float x, float z) = 0;
		glm::vec3 getNormal(float x, float z, float delta);

};

class HeightMap: public BoundingBox  {
	private: 
		float step;
		HeightFunction * func;
	public: 
		HeightMap(HeightFunction * func, glm::vec3 min, glm::vec3 max, float step);
		
   		glm::vec3 getNormalAt(float x, float z);
		glm::vec3 getPoint(BoundingCube cube);

		glm::ivec2 getIndexes(float x, float z);
		glm::vec2 getHeightRangeBetween(BoundingCube cube);
		bool hitsBoundary(BoundingCube cube);
		bool isContained(BoundingCube p);
		bool contains(glm::vec3 point);
		ContainmentType test(BoundingCube cube);

};

class ContainmentHandler {
	public: 
		virtual ContainmentType check(BoundingCube cube) = 0;
		virtual Vertex getVertex(BoundingCube cube, ContainmentType solid) = 0;
		virtual glm::vec3 getCenter() = 0;
		virtual bool contains(glm::vec3 p) = 0;
		virtual bool isContained(BoundingCube cube) = 0;
};


class OctreeNode {
	public: 
		Vertex vertex;
		OctreeNode *children[8];
		bool simplified;
		uint mask;
		ContainmentType solid;
		void * info;
		int infoType;
		OctreeNode(Vertex vertex);
		~OctreeNode();
		void clear();
		bool isEmpty();
		void setChild(int i, OctreeNode * node);
};

class IteratorHandler {
	public: 
		virtual bool test(int level, OctreeNode * node, BoundingCube cube, void * context) = 0;
		virtual void * before(int level, OctreeNode * node, BoundingCube cube, void * context) = 0;
		virtual void after(int level, OctreeNode * node, BoundingCube cube, void * context) = 0;
		virtual OctreeNode * getChild(OctreeNode * node, int index) =0;
};


class Octree: public BoundingCube {
	public: 
		float minSize;
		int geometryLevel;
		OctreeNode * root;

		Octree(float minSize, int geometryLevel);
		void expand(ContainmentHandler * handler);
		void add(ContainmentHandler * handler);
		void del(ContainmentHandler * handler);
		void iterate(IteratorHandler * handler);
		OctreeNode * getNodeAt(glm::vec3 pos, int level, bool simplify);
		std::vector<OctreeNode*> getNodeCorners(BoundingCube cube, int level);
		std::vector<OctreeNode*> getQuadNodes(std::vector<OctreeNode*> corners, glm::ivec4 quad);
		std::vector<OctreeNode*> getNeighbors(BoundingCube cube, int level);

		void save(std::string filename);
		static glm::vec3 getShift(int i);
		static glm::vec3 getShift3(int i);

		int getHeight(BoundingCube cube);

};

class Tesselator : public IteratorHandler{
	public:
		Octree * tree;
		long triangles = 0;

		Tesselator(Octree * tree);
		void * before(int level, OctreeNode * node, BoundingCube cube, void * context);
		void after(int level, OctreeNode * node, BoundingCube cube, void * context);
		bool test(int level, OctreeNode * node, BoundingCube cube, void * context);
		OctreeNode * getChild(OctreeNode * node, int index);
};

class Geometry
{
private:
	/* data */
public:
	std::vector<Vertex> vertices;
	std::vector<uint> indices;
	std::map <std::string, int> compactMap;

	Geometry(/* args */);
	~Geometry();

	Vertex * addVertex(Vertex vertex, bool textureUnique);
};

class SphereGeometry : public Geometry{
    int lats;
    int longs;
public:
	SphereGeometry(int lats, int longs);

};


class Frustum
{
public:
	Frustum() {}

	// m = ProjectionMatrix * ViewMatrix 
	Frustum(glm::mat4 m);

	// http://iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
	bool isBoxVisible(BoundingBox box);

private:
	enum Planes
	{
		Left = 0,
		Right,
		Bottom,
		Top,
		Near,
		Far,
		Count,
		Combinations = Count * (Count - 1) / 2
	};

	template<Planes i, Planes j>
	struct ij2k
	{
		enum { k = i * (9 - i) / 2 + j - 1 };
	};

	template<Planes a, Planes b, Planes c> glm::vec3 intersection(glm::vec3* crosses);
	
	glm::vec4   m_planes[Count];
	glm::vec3   m_points[8];
};

class TextureBrush {
	public:
	virtual void paint(Vertex * v) = 0;
};


class SphereContainmentHandler : public ContainmentHandler {
	public:
	BoundingSphere sphere;
    TextureBrush * brush;

	SphereContainmentHandler(BoundingSphere s, TextureBrush * b);
	glm::vec3 getCenter();
	bool contains(glm::vec3 p);
	bool isContained(BoundingCube p);
	glm::vec3 getNormal(glm::vec3 pos);
	ContainmentType check(BoundingCube cube);
	Vertex getVertex(BoundingCube cube, ContainmentType solid);
};

class BoxContainmentHandler : public ContainmentHandler {
	public: 
	BoundingBox box;
    TextureBrush * brush;

	BoxContainmentHandler(BoundingBox box, TextureBrush * b);
	glm::vec3 getCenter();
	bool contains(glm::vec3 p);
	bool isContained(BoundingCube p);
	ContainmentType check(BoundingCube cube);
	Vertex getVertex(BoundingCube cube, ContainmentType solid);
};

class HeightMapContainmentHandler : public ContainmentHandler {
	public: 
	HeightMap * map;
    TextureBrush * brush;

	HeightMapContainmentHandler(HeightMap * m, TextureBrush * b);
	glm::vec3 getCenter();
	bool contains(glm::vec3 p);
	bool isContained(BoundingCube p);
	float intersection(glm::vec3 a, glm::vec3 b);
	glm::vec3 getNormal(glm::vec3 pos);
	ContainmentType check(BoundingCube cube);
	Vertex getVertex(BoundingCube cube, ContainmentType solid);
};


class Math
{
public:
	Math();
	~Math();
	static bool isBetween(float x, float min, float max);	
	static int clamp(int val, int min, int max);
	static float clamp(float val, float min, float max);
	static int triplanarPlane(glm::vec3 position, glm::vec3 normal);
	static int mod(int a, int b);
	static glm::vec2 triplanarMapping(glm::vec3 position, int plane);
	static glm::vec3 surfaceNormal(glm::vec3 point, BoundingBox box);

};


#endif

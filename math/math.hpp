#ifndef MATH_HPP
#define MATH_HPP

#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <map>

enum ContainmentType {
	Contains,
	Intersects,
	Disjoint,
	IsContained
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    int texIndex;
    glm::vec3 tangent;
    glm::vec3 bitangent;

    Vertex(){
		this->position = glm::vec3(0);
    	this->normal = glm::vec3(0);
    	this->texCoord = glm::vec2(0);
    	this->texIndex = 0;
    }

    Vertex(glm::vec3 pos) {
    	this->position = pos;
    	this->normal = glm::vec3(0);
    	this->texCoord = glm::vec2(0);
    	this->texIndex = 0;
    }

    Vertex(glm::vec3 pos,glm::vec3 normal,glm::vec2 texCoord, uint texIndex) {
    	this->position = pos;
    	this->normal = normal;
    	this->texCoord = texCoord;
    	this->texIndex = texIndex;
    }


    std::string toKey() {
		std::stringstream ss;
		ss << "[" << position.x <<  " " <<  position.y <<  " " <<  position.z << " " << texCoord.x <<  " " <<  texCoord.y <<"]";
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
};


class BoundingSphere {
	public: 
		glm::vec3 center;
		float radius;
		BoundingSphere();		
		BoundingSphere(glm::vec3 center, float radius);
		bool contains(glm::vec3 point);
		ContainmentType contains(BoundingCube cube);
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
		ContainmentType contains(BoundingCube cube);
		bool intersects(BoundingSphere sphere);
		bool contains(glm::vec3 point);
};

class HeightMap {
	private: 
		int height;
		int width;
		glm::vec3 min;
		glm::vec3 max;
		std::vector<std::vector<float>> data; 

	public: 
		HeightMap();
		HeightMap(glm::vec3 min, glm::vec3 max, int width, int height);
		
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
		float getHeightAt(float x, float z);
		glm::vec3 getPointAt(BoundingCube cube);
		
		float getData(int x, int z);
		glm::ivec2 getIndexes(float x, float z);
		glm::vec2 getHeightRangeBetween(BoundingCube cube);
		bool hitsBoundary(BoundingCube cube);

		bool contains(glm::vec3 point);
		ContainmentType contains(BoundingCube cube);

};

class ContainmentHandler {
	public: 
		virtual ContainmentType check(BoundingCube cube, Vertex * vertex) = 0;
		virtual glm::vec3 getCenter() = 0;
};

class SphereContainmentHandler : public ContainmentHandler {

	public:
		BoundingSphere sphere;
		unsigned char texture;

		SphereContainmentHandler(BoundingSphere sphere, unsigned char texture);
		glm::vec3 getCenter();
		ContainmentType check(BoundingCube cube, Vertex * vertex);
};

class BoxContainmentHandler : public ContainmentHandler {

	public:
		BoundingBox box;
		unsigned char texture;

		BoxContainmentHandler(BoundingBox box, unsigned char texture);
		glm::vec3 getCenter();
		ContainmentType check(BoundingCube cube, Vertex * vertex);
};

class OctreeNode {
	public: 
		Vertex vertex;
		ContainmentType solid;
		OctreeNode *children[8];
		OctreeNode(Vertex vertex);
		~OctreeNode();
		void clear();
		void setChild(int i, OctreeNode * node);
};

class IteratorHandler {
	public: 
		virtual int iterate(int level, OctreeNode * node, BoundingCube cube) = 0;
};

class TesselatorHandler {
	public: 
		std::vector<Vertex> vertices;
		std::vector<uint> indices;

		virtual int iterate(int level, OctreeNode * node, BoundingCube cube) = 0;
};

class Octree: public BoundingCube {
	public: 
		float minSize;
		OctreeNode * root;

		Octree(float minSize);
		void expand(ContainmentHandler * handler);
		void add(ContainmentHandler * handler);
		void del(ContainmentHandler * handler);
		void iterate(IteratorHandler * handler);
		OctreeNode * getNodeAt(glm::vec3 pos, int level);
		void save(std::string filename);
		static glm::vec3 getShift(int i);
		int getHeight(BoundingCube cube);
};

class Tesselator : public TesselatorHandler{
	Octree * tree;
	std::map <std::string, int> compactMap;

	public:
		Tesselator(Octree * tree);
		int iterate(int level, OctreeNode * node, BoundingCube cube);
		void addVertex(Vertex vertex);

};

class Math
{
public:
	Math();
	~Math();
	static bool isBetween(float x, float min, float max);	
	static int clamp(int val, int min, int max);
	static float clamp(float val, float min, float max);


};


#endif

#include "math/math.hpp"
#include "gl/gl.hpp"

class SphereContainmentHandler : public ContainmentHandler {
	public:
	BoundingSphere sphere;
    BrushHandler * painter;

	SphereContainmentHandler(BoundingSphere s, BrushHandler * b) : ContainmentHandler(){
		this->sphere = s;
		this->painter = b;
	}

	glm::vec3 getCenter() {
		return sphere.center;
	}

	bool contains(glm::vec3 p) {
		return sphere.contains(p);
	}

	bool isContained(BoundingCube p) {
		return p.contains(sphere);
	}

	glm::vec3 getNormal(glm::vec3 pos) {
		return glm::normalize( pos - sphere.center);
	}

	ContainmentType check(BoundingCube cube) {
		return sphere.test(cube); 
	}

	Vertex getVertex(BoundingCube cube, ContainmentType solid) {
		Vertex vertex(cube.getCenter());
		glm::vec3 c = this->sphere.center;
		float r = this->sphere.radius;
		glm::vec3 a = cube.getCenter();
		glm::vec3 n = glm::normalize(a-c);
		glm::vec3 p = c + n*r;
		vertex.position = glm::clamp(p, cube.getMin(), cube.getMax());
		vertex.normal = getNormal(vertex.position);

        painter->paint(&vertex);
		return vertex;
	}

};

class BoxContainmentHandler : public ContainmentHandler {
	public: 
	BoundingBox box;
    BrushHandler * painter;

	BoxContainmentHandler(BoundingBox box, BrushHandler * b) : ContainmentHandler(){
		this->box = box;
		this->painter = b;
	}

	glm::vec3 getCenter() {
		return box.getCenter();
	}

	bool contains(glm::vec3 p) {
		return box.contains(p);
	}

	bool isContained(BoundingCube p) {
		return p.contains(box);
	}
	
	ContainmentType check(BoundingCube cube) {
		return box.test(cube); 
	}

	Vertex getVertex(BoundingCube cube, ContainmentType solid) {
		Vertex vertex(cube.getCenter());

		glm::vec3 min = this->box.getMin();
		glm::vec3 max = this->box.getMax();
		glm::vec3 c = cube.getCenter();
	
		vertex.position = glm::clamp(c, min, max);
		vertex.normal = Math::surfaceNormal(vertex.position, box);
        painter->paint(&vertex);	
		return vertex;
	}
};

class HeightMapContainmentHandler : public ContainmentHandler {
	public: 
	HeightMap * map;
    BrushHandler * painter;

	HeightMapContainmentHandler(HeightMap * m, BrushHandler * b) : ContainmentHandler(){
		this->map = m;
		this->painter = b;
	}

	glm::vec3 getCenter() {
		return map->getCenter();
	}

	bool contains(glm::vec3 p) {
		return map->contains(p);
	}

	bool isContained(BoundingCube p) {
		return map->isContained(p);
	}

	float intersection(glm::vec3 a, glm::vec3 b) {
		return 0;	
	} 

	glm::vec3 getNormal(glm::vec3 pos) {
		return map->getNormalAt(pos.x, pos.z);
	}

	ContainmentType check(BoundingCube cube) {
		return map->test(cube); 
	}

	Vertex getVertex(BoundingCube cube, ContainmentType solid) {
		Vertex vertex(cube.getCenter());

		if(map->hitsBoundary(cube)) {
			vertex.normal = Math::surfaceNormal(cube.getCenter(), *map);
			glm::vec3 c = cube.getCenter()+vertex.normal*cube.getLength();
			c = glm::clamp(c, map->getMin(), map->getMax() );
			c = glm::clamp(c,cube.getMin(), cube.getMax() );
			vertex.position = c;
		} else {
			glm::vec3 c = glm::clamp(map->getPoint(cube), map->getMin(), map->getMax());
			vertex.position = c;
			vertex.normal = getNormal(vertex.position);
		}

        painter->paint(&vertex);
		return vertex;
	}
};

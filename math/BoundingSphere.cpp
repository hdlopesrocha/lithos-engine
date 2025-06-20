#include "math.hpp"
#include "SDF.hpp"

BoundingSphere::BoundingSphere(glm::vec3 center, float radius) {
	this->center = center;
	this->radius = radius;
}

BoundingSphere::BoundingSphere() {
	this->center = glm::vec3(0,0,0);
	this->radius = 0;
}

bool BoundingSphere::contains(const glm::vec3 point) const {
	glm::vec3 temp = point - this->center;
	return glm::dot(temp, temp) < radius*radius;
}

bool BoundingSphere::intersects(const AbstractBoundingBox& cube) const {
    float squaredDistance = Math::squaredDistPointAABB( center, cube.getMin(), cube.getMax() );
    return squaredDistance <= (radius * radius);
}

ContainmentType BoundingSphere::test(const AbstractBoundingBox& cube) const {
    // Classify corners
    unsigned char mask = 0;

    for(int i=0; i < 8; ++i) {
        glm::vec3 point(cube.getMin()+ AbstractBoundingBox::getShift(i)*cube.getLengthX());
        if(contains(point)){
            mask |= (1 << i);
        }
    } 

    // Classifify type
    if(mask == 0xff) {
        return ContainmentType::Contains;
    } else if(mask > 0 || intersects(cube)) {
        return ContainmentType::Intersects;
    }
    
    return ContainmentType::Disjoint;
}

SphereContainmentHandler::SphereContainmentHandler(BoundingSphere s) : ContainmentHandler(), sphere(s) {

}

glm::vec3 SphereContainmentHandler::getCenter() const {
    return sphere.center;
}

bool SphereContainmentHandler::isContained(const BoundingCube &cube) const {
    return cube.contains(sphere);
}

ContainmentType SphereContainmentHandler::check(const BoundingCube &cube) const {
    return sphere.test(cube); 
}

Vertex SphereContainmentHandler::getVertex(const BoundingCube &cube, glm::vec3 previousPoint) const {
    glm::vec3 c = this->sphere.center;
    float r = this->sphere.radius;
    glm::vec3 a = previousPoint;
    glm::vec3 n = glm::normalize(a-c);
    glm::vec3 p = c + n*r;

    Vertex vertex(p);
    vertex.normal = Math::surfaceNormal(vertex.position, this->sphere);

    return vertex;
}

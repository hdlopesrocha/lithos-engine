#include "math.hpp"

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

   float check(float pn, float bmin,
         float bmax ) 
    {
        float out = 0;
        float v = pn;
 
        if ( v < bmin ) 
        {             
            float val = (bmin - v);             
            out += val * val;         
        }         
         
        if ( v > bmax )
        {
            float val = (v - bmax);
            out += val * val;
        }
 
        return out;
    };
 


float squaredDistPointAABB( const glm::vec3 p, const AbstractBoundingBox& aabb )
{
 
    // Squared distance
    float sq = 0.0;
 
    sq += check( p[0], aabb.getMin()[0], aabb.getMax()[0] );
    sq += check( p[1], aabb.getMin()[1], aabb.getMax()[1] );
    sq += check( p[2], aabb.getMin()[2], aabb.getMax()[2] );
 
    return sq;
}


bool BoundingSphere::intersects(const AbstractBoundingBox& cube) const {
    float squaredDistance = squaredDistPointAABB( center, cube );
    return squaredDistance <= (radius * radius);
}

ContainmentType BoundingSphere::test(const AbstractBoundingBox& cube) const {
    // Classify corners
    unsigned char mask = 0;

    for(int i=0; i < 8; ++i) {
        glm::vec3 point(cube.getMin()+ Octree::getShift(i)*cube.getLengthX());
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


SphereContainmentHandler::SphereContainmentHandler(BoundingSphere s, const TexturePainter &b) : ContainmentHandler(), sphere(s), brush(b){

}

glm::vec3 SphereContainmentHandler::getCenter() const {
    return sphere.center;
}

bool SphereContainmentHandler::contains(const glm::vec3 p) const {
    return sphere.contains(p);
}

bool SphereContainmentHandler::isContained(const BoundingCube &cube) const {
    return cube.contains(sphere);
}

glm::vec3 SphereContainmentHandler::getNormal(const glm::vec3 pos) const  {
    return glm::normalize( pos - sphere.center);
}

ContainmentType SphereContainmentHandler::check(const BoundingCube &cube) const {
    return sphere.test(cube); 
}

Vertex SphereContainmentHandler::getVertex(const BoundingCube &cube, ContainmentType solid, glm::vec3 previousPoint) const {
    glm::vec3 c = this->sphere.center;
    float r = this->sphere.radius;
    glm::vec3 a = previousPoint;
    glm::vec3 n = glm::normalize(a-c);
    glm::vec3 p = c + n*r;

    Vertex vertex(p);
    vertex.normal = getNormal(vertex.position);

    brush.paint(vertex);
    return vertex;
}

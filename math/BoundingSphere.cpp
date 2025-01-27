#include "math.hpp"

BoundingSphere::BoundingSphere(glm::vec3 center, float radius) {
	this->center = center;
	this->radius = radius;
}

BoundingSphere::BoundingSphere() {
	this->center = glm::vec3(0,0,0);
	this->radius = 0;
}

bool BoundingSphere::contains(glm::vec3 point){
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
 


float squaredDistPointAABB( glm::vec3 p, BoundingCube aabb )
{
 
    // Squared distance
    float sq = 0.0;
 
    sq += check( p[0], aabb.getMin()[0], aabb.getMax()[0] );
    sq += check( p[1], aabb.getMin()[1], aabb.getMax()[1] );
    sq += check( p[2], aabb.getMin()[2], aabb.getMax()[2] );
 
    return sq;
}


bool BoundingSphere::intersects(BoundingCube cube) {
    float squaredDistance = squaredDistPointAABB( center, cube );
    return squaredDistance <= (radius * radius);
}

ContainmentType BoundingSphere::test(BoundingCube cube) {
    // Classify corners
    unsigned char mask = 0;

    for(int i=0; i < 8; ++i) {
        glm::vec3 point(cube.getMin()+ Octree::getShift(i)*cube.getLength());
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


SphereContainmentHandler::SphereContainmentHandler(BoundingSphere s, BrushHandler * b) : ContainmentHandler(){
    this->sphere = s;
    this->painter = b;
}

glm::vec3 SphereContainmentHandler::getCenter() {
    return sphere.center;
}

bool SphereContainmentHandler::contains(glm::vec3 p) {
    return sphere.contains(p);
}

bool SphereContainmentHandler::isContained(BoundingCube p) {
    return p.contains(sphere);
}

glm::vec3 SphereContainmentHandler::getNormal(glm::vec3 pos) {
    return glm::normalize( pos - sphere.center);
}

ContainmentType SphereContainmentHandler::check(BoundingCube cube) {
    return sphere.test(cube); 
}

Vertex SphereContainmentHandler::getVertex(BoundingCube cube, ContainmentType solid) {
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

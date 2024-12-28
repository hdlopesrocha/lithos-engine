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

ContainmentType BoundingSphere::contains(BoundingCube cube) {



    ContainmentType result;
    result = ContainmentType::Disjoint;
    
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
        result = ContainmentType::Contains;
    }
    else if(mask > 0) {
        result = ContainmentType::Intersects;
    }
    else {        
        glm::vec3 minSphere = (center-glm::vec3(radius));
        glm::vec3 maxSphere = (center+glm::vec3(radius));
        
        if ( cube.getMin()[0] <=  minSphere[0] && maxSphere[0] <= cube.getMax()[0] && 
             cube.getMin()[1] <=  minSphere[1] && maxSphere[1] <= cube.getMax()[1] && 
             cube.getMin()[2] <=  minSphere[2] && maxSphere[2] <= cube.getMax()[2] ) {
            result = ContainmentType::IsContained;
        } else if(intersects(cube)) {
            result = ContainmentType::Intersects;
        }
    }  

    return result;
}

SphereContainmentHandler::SphereContainmentHandler(BoundingSphere s, unsigned char t) : ContainmentHandler(){
    this->sphere = s;
    this->texture = t;
}

glm::vec3 SphereContainmentHandler::getCenter() {
    return sphere.center;
}

ContainmentType SphereContainmentHandler::check(BoundingCube cube, Vertex * vertex) {
    ContainmentType result = sphere.contains(cube); 

    if(result == ContainmentType::Intersects) {
        glm::vec3 c = this->sphere.center;
        float r = this->sphere.radius;
        glm::vec3 a = cube.getCenter();
        glm::vec3 n = glm::normalize(a-c);
        glm::vec3 p = glm::clamp(c + n*r, cube.getMin(), cube.getMax());

        vertex->normal = n;
        vertex->position = p;
        vertex->texIndex = this->texture;
    }
    return result;
}
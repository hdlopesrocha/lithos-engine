#include "math.hpp"


HeightMap::HeightMap(HeightFunction * func, glm::vec3 min, glm::vec3 max, float step) : BoundingBox(min, max){
    this->func = func;
    this->step = step;
}


glm::vec2 HeightMap::getHeightRangeBetween(BoundingCube cube) {
    glm::vec2 range = glm::vec2(func->getHeightAt(cube.getCenter().x,0,cube.getCenter().z));

    for(float x = cube.getMinX() ; x <= cube.getMaxX(); x+=step) {
        for(float z = cube.getMinZ() ; z <= cube.getMaxZ(); z+=step) {
            float h = func->getHeightAt(x,0,z);
            range[0] = h < range[0] ? h : range[0];
            range[1] = h > range[1] ? h : range[1];
        }        
    }

    return range;
}

glm::vec3 HeightFunction::getNormal(float x, float z, float delta) {
    float q11 = getHeightAt(x, 0,z);
    float q21 = getHeightAt(x+delta,0, z);
    float q12 = getHeightAt(x,0, z+delta);

    glm::vec3 v11 = glm::vec3(0, q11, 0);
    glm::vec3 v21 = glm::vec3(delta, q21, 0);
    glm::vec3 v12 = glm::vec3(0, q12, delta);

    glm::vec3 n21 = glm::normalize(v21 -v11 );
    glm::vec3 n12 = glm::normalize(v12 -v11 );

    return glm::cross(n12,n21);
}

glm::vec3 HeightMap::getNormalAt(float x, float z) {
    return func->getNormal(x,z,step);
}

glm::vec3 getShift(int i) {
	return glm::vec3( ((i >> 0) % 2) , ((i >> 2) % 2) , ((i >> 1) % 2));
}

glm::vec3 HeightMap::getPoint(BoundingCube cube) {
    glm::vec3 v = cube.getCenter();
    float h = func->getHeightAt(v.x,0,v.z);
    if( Math::isBetween(h, cube.getMinY(), cube.getMaxY())){
        return glm::vec3(v.x, h, v.z);
    }  
   
    for(int i =0; i < 4 ; ++i) {
        v = cube.getMin() + cube.getLength() * getShift(i);
        h = func->getHeightAt(v.x,0,v.z);
        if( Math::isBetween(h, cube.getMinY(), cube.getMaxY())){
            return glm::vec3(v.x, h, v.z);
        }
    }
    return cube.getCenter();
}

bool HeightMap::contains(glm::vec3 point){
    BoundingBox box(getMin(), getMax());
    float h = func->getHeightAt(point.x, 0, point.z);

    return box.contains(point) && Math::isBetween(point.y, getMinY(), h);
}

bool HeightMap::isContained(BoundingCube p) {
    BoundingBox box(getMin(), getMax());
    return p.contains(box);
}
    
bool HeightMap::hitsBoundary(BoundingCube cube) {
    BoundingBox box(getMin(), getMax());

    ContainmentType result = box.test(cube);
    bool allPointsUnderground = true;

    glm::vec2 h = getHeightRangeBetween(cube);
    for(int i = 0; i<8 ; ++i) {
        glm::vec3 p = cube.getMin() + cube.getLength() * getShift(i);
        if(h[0] <= p.y) {
            allPointsUnderground = false;
            break;
        }
    }

    return result == ContainmentType::Intersects && allPointsUnderground;
}

ContainmentType HeightMap::test(BoundingCube cube) {
    BoundingBox box(getMin(), getMax());

    ContainmentType result = box.test(cube);

    if(result != ContainmentType::Disjoint) {
        glm::vec2 range = getHeightRangeBetween(cube);
     //   std::cout << range[0] << " ! " << range[1] << std::endl;
        BoundingBox minBox(getMin(), glm::vec3(getMax().x, range[0], getMax().z ));
        BoundingBox maxBox(getMin(), glm::vec3(getMax().x, range[1], getMax().z ));
        
        ContainmentType minResult = minBox.test(cube);
        ContainmentType maxResult = maxBox.test(cube);
       
        if(minResult == ContainmentType::Contains){       
            result = ContainmentType::Contains;
        } else if(maxResult == ContainmentType::Disjoint){
            result = ContainmentType::Disjoint;
        } else {
            result = ContainmentType::Intersects;
        }
   }

    return result;
}


HeightMapContainmentHandler::HeightMapContainmentHandler(HeightMap * m, TextureBrush * b) : ContainmentHandler(){
    this->map = m;
    this->brush = b;
}

glm::vec3 HeightMapContainmentHandler::getCenter() {
    return map->getCenter();
}

bool HeightMapContainmentHandler::contains(glm::vec3 p) {
    return map->contains(p);
}

bool HeightMapContainmentHandler::isContained(BoundingCube p) {
    return map->isContained(p);
}

float HeightMapContainmentHandler::intersection(glm::vec3 a, glm::vec3 b) {
    return 0;	
} 

glm::vec3 HeightMapContainmentHandler::getNormal(glm::vec3 pos) {
    return map->getNormalAt(pos.x, pos.z);
}

ContainmentType HeightMapContainmentHandler::check(BoundingCube cube) {
    return map->test(cube); 
}

Vertex HeightMapContainmentHandler::getVertex(BoundingCube cube, ContainmentType solid) {
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

    brush->paint(&vertex);
    return vertex;
}


#include "math.hpp"

Vertex getVertex(glm::vec3 v) {
    return Vertex(v,glm::normalize(v), Math::triplanarMapping(v,0),0 );
}

BoxGeometry::BoxGeometry(const BoundingBox &box) {
    glm::vec3 corners[8] = {
        {box.getMinX(), box.getMinY(), box.getMinZ()},
        {box.getMaxX(), box.getMinY(), box.getMinZ()},
        {box.getMaxX(), box.getMaxY(), box.getMinZ()},
        {box.getMinX(), box.getMaxY(), box.getMinZ()},
        {box.getMinX(), box.getMinY(), box.getMaxZ()},
        {box.getMaxX(), box.getMinY(), box.getMaxZ()},
        {box.getMaxX(), box.getMaxY(), box.getMaxZ()},
        {box.getMinX(), box.getMaxY(), box.getMaxZ()},
    };
    
    uint indices[24] = {
        0, 1, 1, 2, 2, 3, 3, 0, // bottom
        4, 5, 5, 6, 6, 7, 7, 4, // top
        0, 4, 1, 5, 2, 6, 3, 7  // sides
    };

    for(int i=0; i < 24 ; ++i) {
        addVertex(getVertex(corners[indices[i]]));
    }

}


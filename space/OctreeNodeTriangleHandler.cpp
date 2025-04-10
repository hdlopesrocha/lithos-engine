#include "space.hpp"

OctreeNodeTriangleHandler::OctreeNodeTriangleHandler(Geometry * chunk, long * count){
    this->chunk = chunk;
    this->count = count;
}

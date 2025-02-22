#include "math.hpp"

int addTriangle(OctreeNode* c0, OctreeNode* c1, OctreeNode* c2, Geometry * chunk, bool reverse) {

    Vertex v0 = c0->vertex;
	Vertex v1 = c1->vertex;
	Vertex v2 = c2->vertex;

    int count = 0;

	if(c0!= c1 && c1 != c2 && c0!=c2 && c0->vertex.brushIndex>=0 && c1->vertex.brushIndex>=0 && c2->vertex.brushIndex>=0){
		chunk->addVertex(v0, true);
		chunk->addVertex(v2, true);
		chunk->addVertex(v1, true);
		++count;
	}

    return count;

}

QuadNodeHandler::QuadNodeHandler(Geometry * chunk, int * triangles){
    this->chunk = chunk;
    this->triangles = triangles;
}

QuadNodeTesselatorHandler::QuadNodeTesselatorHandler(Geometry * chunk, int * triangles) : QuadNodeHandler(chunk, triangles){
    ++*triangles;
}

void QuadNodeTesselatorHandler::handle(OctreeNode* c0,OctreeNode* c1,OctreeNode* c2, bool sign) {
	*triangles += addTriangle(c0,c1,c2, chunk, sign);
}

QuadNodeInstanceBuilderHandler::QuadNodeInstanceBuilderHandler(Geometry * chunk, int * triangles,OctreeNode ** corners,std::vector<glm::mat4> * matrices) : QuadNodeHandler(chunk, triangles){
    this->corners = corners;
    this-> matrices = matrices;
}

void QuadNodeInstanceBuilderHandler::handle(OctreeNode* c0,OctreeNode* c1,OctreeNode* c2, bool sign){
    glm::mat4 model(1.0);
    model = glm::translate(glm::mat4(1.0), c0->vertex.position);
    matrices->push_back(model);
}

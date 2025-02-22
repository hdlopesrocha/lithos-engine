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

int addQuad(OctreeNode** quad, Geometry * chunk, bool reverse) {
	OctreeNode* c0 = quad[reverse ? 3:0];
	OctreeNode* c1 = quad[reverse ? 2:1];
	OctreeNode* c2 = quad[reverse ? 1:2];
	OctreeNode* c3 = quad[reverse ? 0:3];


	Vertex v0 = c0->vertex;
	Vertex v1 = c1->vertex;
	Vertex v2 = c2->vertex;
	Vertex v3 = c3->vertex;

	float scale = 0.1;
	int plane = Math::triplanarPlane(v0.position, v0.normal);
	v0.texCoord = Math::triplanarMapping(v0.position, plane)*scale;
	v1.texCoord = Math::triplanarMapping(v1.position, plane)*scale;
	v2.texCoord = Math::triplanarMapping(v2.position, plane)*scale;
	v3.texCoord = Math::triplanarMapping(v3.position, plane)*scale;	

	int count = 0;

    count += addTriangle(c0,c1,c2, chunk, reverse);
    count += addTriangle(c0,c2,c3, chunk, reverse);

	//std::cout << "One" << std::endl;
	return count;
}

QuadNodeHandler::QuadNodeHandler(Geometry * chunk, int * triangles){
    this->chunk = chunk;
    this->triangles = triangles;

}

QuadNodeTesselatorHandler::QuadNodeTesselatorHandler(Geometry * chunk, int * triangles) : QuadNodeHandler(chunk, triangles){
    ++*triangles;
}

void QuadNodeTesselatorHandler::handle(OctreeNode** quad, bool sign) {
	*triangles += addQuad(quad, chunk, sign);
}

QuadNodeInstanceBuilderHandler::QuadNodeInstanceBuilderHandler(Geometry * chunk, int * triangles,OctreeNode ** corners,std::vector<glm::mat4> * matrices) : QuadNodeHandler(chunk, triangles){
    this->corners = corners;
    this-> matrices = matrices;
}

void QuadNodeInstanceBuilderHandler::handle(OctreeNode** quad, bool sign){
    OctreeNode * c = quad[0];
    glm::mat4 model(1.0);
    model = glm::translate(glm::mat4(1.0), c->vertex.position);
    matrices->push_back(model);
}

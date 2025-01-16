#include <bitset>
#include "math/math.hpp"
#include "gl/gl.hpp"

class DrawableGeometry {
	public:
	GLuint vao, vbo, ebo;
	int indices;
};


class OctreeRenderer : public IteratorHandler{
	Octree * tree;
	Geometry chunk;
	public: 
		int loaded = 0;

		OctreeRenderer(Octree * tree) {
			this->tree = tree;

		}

		DrawableGeometry * createDrawableGeometry(Geometry * t){
			DrawableGeometry * geo = new DrawableGeometry();
			geo->indices = t->indices.size();
			glGenVertexArrays(1, &geo->vao);
			glGenBuffers(1, &geo->vbo);
			glGenBuffers(1, &geo->ebo);

			glBindVertexArray(geo->vao);
			glBindBuffer(GL_ARRAY_BUFFER, geo->vbo);
			glBufferData(GL_ARRAY_BUFFER, t->vertices.size()*sizeof(Vertex), t->vertices.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, t->indices.size()*sizeof(uint), t->indices.data(), GL_STATIC_DRAW);
			
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoord));
			glEnableVertexAttribArray(2);		
			glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*) offsetof(Vertex, texIndex) );
			glEnableVertexAttribArray(3);
				
			glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, parallaxScale));
			glEnableVertexAttribArray(4);	
			glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, parallaxMinLayers));
			glEnableVertexAttribArray(5);	
			glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, parallaxMaxLayers));
			glEnableVertexAttribArray(6);	
			glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, shininess));
			glEnableVertexAttribArray(7);	
			return geo;
		}



		void * before(int level, OctreeNode * node, BoundingCube cube, void * context) {			
			if(node->info != NULL){
				// just geometry not drawable
				if(node->infoType == 0 && loaded == 0) {
					Geometry * info = (Geometry*) node->info;
					node->info = createDrawableGeometry(info);
					node->infoType = 1;
					++loaded;
					delete info;
				}

				// drawable geometry
				if(node->infoType == 1) {
					DrawableGeometry * info = (DrawableGeometry*) node->info;
					glBindVertexArray(info->vao);
					glDrawElements(GL_PATCHES, info->indices, GL_UNSIGNED_INT, 0);
				}
				
			}
			return NULL; 			 			
		}

		int after(int level, OctreeNode * node, BoundingCube cube, void * context) {			
			return 1;
		}

};


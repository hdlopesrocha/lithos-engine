	#include "math.hpp"

    Vertex getVertex(glm::vec3 v, int plane) {
        return Vertex(v,glm::normalize(v), Math::triplanarMapping(v,plane),0 );
    }

        void SphereGeometry::addTriangle(glm::vec3 a,glm::vec3 b, glm::vec3 c) {
            glm::vec3 v0 = b-a;
            glm::vec3 v1 = c-a;
            glm::vec3 n = glm::cross(v0,v1);
            int plane = Math::triplanarPlane(a, n);
            addVertex(getVertex(a, plane));
            addVertex(getVertex(b, plane));
            addVertex(getVertex(c, plane));


        }

    SphereGeometry::SphereGeometry(int lats, int longs) {
        float PI = glm::pi<float>();

        for(int i = 0; i <= lats; i++) {
            float lat0 = PI * (-0.5 + (float) (i - 1) / lats);
            float z0  = sin(lat0);
            float zr0 =  cos(lat0);

            float lat1 = PI * (-0.5 + (float) i / lats);
            float z1 = sin(lat1);
            float zr1 = cos(lat1);

            for(int j = 0; j <= longs; j++) {
                float lng0 = 2 * PI * (float) (j - 1) / longs;
                float x0 = cos(lng0);
                float y0 = sin(lng0);

                float lng1 = 2 * PI * (float) j / longs;
                float x1 = cos(lng1);
                float y1 = sin(lng1);
            


                glm::vec3 v0 = glm::vec3( x0 * zr0, y0 * zr0, z0 );
                glm::vec3 v1 = glm::vec3( x1 * zr1, y1 * zr1, z1 );
                glm::vec3 v2 = glm::vec3( x0 * zr1, y0 * zr1, z1 );
                glm::vec3 v3 = glm::vec3( x1 * zr0, y1 * zr0, z0 );
            
                addTriangle(v0,v1,v2);
                addTriangle(v0,v3,v1);

        
   



            }
        }

	}


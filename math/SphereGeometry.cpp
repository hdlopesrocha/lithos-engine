	#include "math.hpp"

    Vertex getVertex(float x, float y, float z) {
        return Vertex(glm::vec3(x,y,z),glm::normalize(glm::vec3(x,y,z)),glm::vec2(0),0 );
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

                addVertex(getVertex( x0 * zr0, y0 * zr0, z0 ));
                addVertex(getVertex( x1 * zr1, y1 * zr1, z1 ));
                addVertex(getVertex( x0 * zr1, y0 * zr1, z1 ));


                addVertex(getVertex( x0 * zr0, y0 * zr0, z0 ));
                addVertex(getVertex( x1 * zr0, y1 * zr0, z0 ));
                addVertex(getVertex( x1 * zr1, y1 * zr1, z1 ));



            }
        }

	}


#ifndef SDF_HPP
#define SDF_HPP

#include <glm/glm.hpp>

class SDF
{
public:
	SDF();
	~SDF();
	 
    static float opUnion( float d1, float d2 );
    static float opSubtraction( float d1, float d2 );
    static float opIntersection( float d1, float d2 );
    static float opXor(float d1, float d2 );

    static float opSmoothUnion( float d1, float d2, float k );
    static float opSmoothSubtraction( float d1, float d2, float k );
    static float opSmoothIntersection( float d1, float d2, float k );

	static float box(glm::vec3 p, const glm::vec3 len);
	static float sphere(glm::vec3 p, const float r);

};

#endif
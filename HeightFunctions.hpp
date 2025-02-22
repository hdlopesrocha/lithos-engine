#include "math/math.hpp"
#include <math.h>
#include <glm/gtc/type_ptr.hpp>

class WaveSurface : public HeightFunction {
	float getHeightAt(float x, float z) {
		float amplitude = 10;
		float offset = -36;
		float frequency = 1.0/10.0;

		return offset + amplitude * sin(frequency*x)*cos(frequency*z);
	}
};


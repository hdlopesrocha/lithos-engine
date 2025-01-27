#include "math/math.hpp"
#include <math.h>
#include <glm/gtc/type_ptr.hpp>

#define DB_PERLIN_IMPL
#include "lib/db_perlin.hpp"

class WaveSurface : public HeightFunction {
	float getHeightAt(float x, float z) {
		float amplitude = 10;
		float offset = -36;
		float frequency = 1.0/10.0;

		return offset + amplitude * sin(frequency*x)*cos(frequency*z);
	}
};

class PerlinSurface : public HeightFunction {
	public:
	float amplitude;
	float frequency;
	float offset;


	PerlinSurface(float amplitude, float frequency, float offset) {
		this->amplitude = amplitude;
		this->frequency = frequency;
		this->offset = offset;
	}

	float getHeightAt(float x, float z) {
		float noise = db::perlin(double(x) * frequency, double(z) *frequency);
		return offset + amplitude * noise;
	}
};


class FractalPerlinSurface : public HeightFunction {
	public:
	float amplitude;
	float frequency;
	float offset;

	FractalPerlinSurface(float amplitude, float frequency, float offset) {
		this->amplitude = amplitude;
		this->frequency = frequency;
		this->offset = offset;
	}

	float getHeightAt(float x, float z) {
		float noise = 0;
		float weight = 1.0;
		float total = 0.0;
		float f = frequency;
		int octaves = 8;
		for(int o = 0 ; o < octaves ; ++o) {
			PerlinSurface perlin(1.0, f, 0.0);
			noise += perlin.getHeightAt(x,z) * weight;
			total += weight;
			weight *= 0.5;
			f *= 2;
		}

		noise /= total;

		return offset + amplitude * noise;
	}
};

class GradientPerlinSurface : public HeightFunction {
	public:
	float amplitude;
	float frequency;
	float offset;

	GradientPerlinSurface(float amplitude, float frequency, float offset) {
		this->amplitude = amplitude;
		this->frequency = frequency;
		this->offset = offset;
	}

	float getHeightAt(float x, float z) {
		float noise = 0;
		float weight = 1.0;
		float total = 0.0;
		float f = frequency;

		for(int i = 0 ; i < 6 ; ++i) {
			PerlinSurface perlin(1 , f, 0);
			glm::vec3 n = perlin.getNormal(x,z, 0.5);
		
			float m = 1.0f -Math::clamp(glm::abs(glm::dot(glm::vec3(0,1,0), n)), 0.0f, 1.0f);
			float s = glm::pow(glm::e<float>(),- 1280.0f*m);


			noise += s*perlin.getHeightAt(x,z) * weight;
			total +=  weight;
			weight *= 0.5;

			f *= 2;
		}
		
		noise /= total;


		float beachLevel = 0.1;
		float divisions = 3;
		// Create beach
		if(noise < 0.1){
			noise = beachLevel+ (noise - beachLevel) / divisions;
		}


		return offset + amplitude * noise;
	}
};


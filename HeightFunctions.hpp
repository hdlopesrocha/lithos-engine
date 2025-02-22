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


class CachedHeightMapSurface : public HeightFunction {
	public:
		std::vector<std::vector<float>> data; 
		BoundingBox box;
		float delta;
		int width;
		int height;


	CachedHeightMapSurface(HeightFunction * function, BoundingBox box, float delta) {
		this->box = box;
		this->delta = delta;
		glm::vec3 len = box.getLength();
		this->width = len.x / delta;
		this->height = len.z / delta;
	    this->data = std::vector<std::vector<float>>(width, std::vector<float>(height)); 

		for(int i=0; i<width; ++i) {
			for(int j=0; j<height; ++j) {
				float x = i * delta + box.getMinX();
				float z = j * delta + box.getMinZ();
	            this->data[i][j] = function->getHeightAt(x, 0, z);
			}	
		}
	}

	float getData(int x, int z) {
		return this->data[Math::clamp(x, 0, this->width-1)][Math::clamp(z, 0, this->height-1)];
	}

	float getHeightAt(float x, float yy, float z) {
		// bilinear interpolation
		glm::vec3 len = box.getLength();


		float px = Math::clamp((x-box.getMinX())/len.x, 0.0, 1.0);
		float pz = Math::clamp((z-box.getMinZ())/len.z, 0.0, 1.0);
		int ix = floor(px * width);
		int iz = floor(pz * height);

		float qx = (px * width) - ix;
		float qz = (pz * height) - iz;

		float q11 = getData(ix, iz);
		float q21 = getData(ix+1, iz);
		float q12 = getData(ix, iz+1);
		float q22 = getData(ix+1, iz+1);

		float y1 = (1.0 - qx)*q11 + (qx)*q21;
		float y2 = (1.0 - qx)*q12 + (qx)*q22;

		float y = (1.0 - qz)*y1 + (qz)*y2;

		return y;
	}

};
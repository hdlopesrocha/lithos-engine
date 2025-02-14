#include "math.hpp"
#include <zlib.h>

bool Math::isBetween(float x, float min, float max) {
	return min <= x && x <= max;
}

int Math::clamp(int val, int min, int max) {
	return val < min ? min : val > max ? max : val;
}

float Math::clamp(float val, float min, float max) {
	return val < min ? min : val > max ? max : val;
}

int Math::mod(int a, int b) {
    return (a % b + b) % b;
}

int Math::triplanarPlane(glm::vec3 position, glm::vec3 normal) {
    glm::vec3 absNormal = glm::abs(normal);
    if (absNormal.x > absNormal.y && absNormal.x > absNormal.z) {
        return normal.x > 0 ? 0 : 1;
    } else if (absNormal.y > absNormal.x && absNormal.y > absNormal.z) {
        return normal.y > 0 ? 2 : 3;
    } else {
        return normal.z > 0 ? 4 : 5;
    }
}

glm::vec2 Math::triplanarMapping(glm::vec3 position, int plane) {
    switch (plane) {
        case 0: return glm::vec2(-position.z, -position.y);
        case 1: return glm::vec2(position.z, -position.y);
        case 2: return glm::vec2(position.x, position.z);
        case 3: return glm::vec2(position.x, -position.z);
        case 4: return glm::vec2(position.x, -position.y);
        case 5: return glm::vec2(-position.x, -position.y);
        default: return glm::vec2(0.0,0.0);
    }
}


glm::vec3 Math::surfaceNormal(glm::vec3 point, BoundingBox box) {


    glm::vec3 d = (point - box.getCenter())/box.getLength(); // Vector from center to the point
    glm::vec3 ad = glm::abs(d); // Absolute values of components

    glm::vec3 v = glm::vec3(0);
    // Determine the dominant axis
    if (ad.x >= ad.y && ad.x >= ad.z) {
        v+= glm::vec3((d.x > 0? 1.0f : -1.0f), 0.0f, 0.0f); // Normal along X
    } 
    
    if (ad.y >= ad.x && ad.y >= ad.z) {
        v+= glm::vec3(0.0f, (d.y > 0? 1.0f : -1.0f), 0.0f); // Normal along Y
    } 
    
    if (ad.z >= ad.x && ad.z >= ad.y) {
        v+= glm::vec3(0.0f, 0.0f, (d.z > 0? 1.0f : -1.0f)); // Normal along Z
    }

    return glm::normalize(v);
}

void ensureFolderExists(const std::string& folder) {
    if (!std::filesystem::exists(folder)) {
        std::filesystem::create_directories(folder);
    }
}


std::stringstream gzipDecompressFromIfstream(std::ifstream& inputFile) {
    if (!inputFile) {
        throw std::runtime_error("Failed to open input file.");
    }

    z_stream strm = {};
    if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK) {
        throw std::runtime_error("Failed to initialize zlib for decompression.");
    }

    std::stringstream decompressedStream;
    std::vector<char> inBuffer(1024);
    std::vector<char> outBuffer(1024);

    int ret;
    do {
        // Read compressed data in chunks
        inputFile.read(inBuffer.data(), inBuffer.size());
        strm.next_in = reinterpret_cast<Bytef*>(inBuffer.data());
        strm.avail_in = inputFile.gcount();  // Number of bytes read

        do {
            strm.next_out = reinterpret_cast<Bytef*>(outBuffer.data());
            strm.avail_out = outBuffer.size();

            ret = inflate(&strm, Z_NO_FLUSH);

            if (ret < 0) {
                inflateEnd(&strm);
                throw std::runtime_error("Decompression failed: inflate() error " + std::to_string(ret));
            }

            decompressedStream.write(outBuffer.data(), outBuffer.size() - strm.avail_out);
        } while (strm.avail_out == 0);

    } while (!inputFile.eof() && ret != Z_STREAM_END);

    inflateEnd(&strm);
    return decompressedStream;
}

void gzipCompressToOfstream(std::istream& inputStream, std::ofstream& outputFile) {
    if (!outputFile) {
        throw std::runtime_error("Failed to open output file.");
    }

    z_stream strm = {};
    if (deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        throw std::runtime_error("Failed to initialize zlib for compression.");
    }

    std::vector<char> inBuffer(1024);
    std::vector<char> outBuffer(1024);

    int ret;
    do {
        inputStream.read(inBuffer.data(), inBuffer.size());
        strm.next_in = reinterpret_cast<Bytef*>(inBuffer.data());
        strm.avail_in = inputStream.gcount();  // Number of bytes read

        int flush = inputStream.eof() ? Z_FINISH : Z_NO_FLUSH;

        do {
            strm.next_out = reinterpret_cast<Bytef*>(outBuffer.data());
            strm.avail_out = outBuffer.size();

            ret = deflate(&strm, flush);

            if (ret < 0) {
                deflateEnd(&strm);
                throw std::runtime_error("Compression failed: deflate() error " + std::to_string(ret));
            }

            outputFile.write(outBuffer.data(), outBuffer.size() - strm.avail_out);
        } while (strm.avail_out == 0);

    } while (ret != Z_STREAM_END);

    deflateEnd(&strm);
}

glm::mat4 Math::getCanonicalMVP(glm::mat4 m) {
	return glm::translate(glm::mat4(1.0f), glm::vec3(0.5)) 
					* glm::scale(glm::mat4(1.0f), glm::vec3(0.5)) 
					* m;
}
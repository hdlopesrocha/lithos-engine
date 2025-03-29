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


glm::vec3 Math::surfaceNormal(const glm::vec3 point, const BoundingBox &box) {


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

    int ret=0;
    do {
        inputFile.read(inBuffer.data(), inBuffer.size());
        strm.next_in = reinterpret_cast<Bytef*>(inBuffer.data());
        strm.avail_in = static_cast<uInt>(inputFile.gcount());

        if (strm.avail_in == 0) {
            break; 
        }

        do {
            strm.next_out = reinterpret_cast<Bytef*>(outBuffer.data());
            strm.avail_out = outBuffer.size();

            ret = inflate(&strm, Z_NO_FLUSH);

            if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                inflateEnd(&strm);
                throw std::runtime_error("Decompression failed: inflate() error " + std::to_string(ret));
            }

            decompressedStream.write(outBuffer.data(), outBuffer.size() - strm.avail_out);
        } while (strm.avail_out == 0);

    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);

    if (ret != Z_STREAM_END) {
        throw std::runtime_error("Decompression finished unexpectedly. inflate() error " + std::to_string(ret));
    }

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

glm::mat4 Math::getRotationMatrixFromNormal(glm::vec3 normal, glm::vec3 target) {
    // Compute rotation axis (cross product)
    glm::vec3 rotationAxis = glm::normalize(glm::cross(normal, target));

    // Compute the angle between the vectors
    float angle = glm::acos(glm::dot(normal, target));

    // Create the rotation matrix
    return glm::rotate(glm::mat4(1.0f), angle, rotationAxis);
}


const double a = 6378137.0;  // WGS84 semi-major axis in meters
const double e2 = 0.00669437999014;  // WGS84 eccentricity squared

// Convert degrees to radians
double Math::degToRad(double degrees) {
    return degrees * (M_PI / 180.0);
}

// WGS 84 to ECEF (XYZ) conversion
void Math::wgs84ToEcef(double lat, double lon, double height, double &X, double &Y, double &Z) {
    // Convert latitude and longitude from degrees to radians
    double phi = degToRad(lat);  // Latitude in radians
    double lambda = degToRad(lon);  // Longitude in radians

    // Compute the radius of curvature in the prime vertical
    double N = a / sqrt(1 - e2 * sin(phi) * sin(phi));

    // Compute the ECEF coordinates
    X = (N + height) * cos(phi) * cos(lambda);
    Y = (N + height) * cos(phi) * sin(lambda);
    Z = ((1 - e2) * N + height) * sin(phi);
}

// Function to create a quaternion from yaw, pitch, roll
glm::quat Math::createQuaternion(float yaw, float pitch, float roll) {
    // Convert degrees to radians
    float yawRad   = glm::radians(yaw);
    float pitchRad = glm::radians(pitch);
    float rollRad  = glm::radians(roll);

    // Create individual axis quaternions
    glm::quat qYaw   = glm::angleAxis(yawRad, glm::vec3(0, 1, 0));  // Rotate around Y
    glm::quat qPitch = glm::angleAxis(pitchRad, glm::vec3(1, 0, 0)); // Rotate around X
    glm::quat qRoll  = glm::angleAxis(rollRad, glm::vec3(0, 0, 1));  // Rotate around Z

    // Apply in Yaw -> Pitch -> Roll order (multiplication applies right to left)
    return qYaw * qPitch * qRoll;
}


glm::quat Math::eulerToQuat(float yaw, float pitch, float roll) {
    // Convert degrees to radians
    float yawRad = glm::radians(yaw);
    float pitchRad = glm::radians(pitch);
    float rollRad = glm::radians(roll);

    // Construct quaternion in correct order (Yaw -> Pitch -> Roll)
    glm::quat qYaw   = glm::angleAxis(yawRad, glm::vec3(0, 1, 0));  // Rotate around Y
    glm::quat qPitch = glm::angleAxis(pitchRad, glm::vec3(1, 0, 0)); // Rotate around X
    glm::quat qRoll  = glm::angleAxis(rollRad, glm::vec3(0, 0, 1));  // Rotate around Z

    return qYaw * qPitch * qRoll; // Yaw first, then Pitch, then Roll
}
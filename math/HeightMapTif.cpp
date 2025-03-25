#include "math.hpp"



void processBand(GDALRasterBand* band, int bandIdx, std::vector<uint16_t> &result, int scale) {
    if (band) {
        std::cout << "Processing Band " << (bandIdx) << std::endl;

    } else {
        std::cerr << "Band " << (bandIdx) << " is missing." << std::endl;
        return;
    }

    GDALDataType dtype = band->GetRasterDataType();

    // Retrieve the NoData value dynamically
    int noDataValue;
     band->GetNoDataValue(&noDataValue);


    int width = band->GetXSize();
    int height = band->GetYSize();

    
    result.resize(width * height);


    if (dtype == GDT_Float32) {
        std::vector<float> buffer(width * height);
        band->RasterIO(GF_Read, 0, 0, width, height, buffer.data(), width, height, GDT_Float32, 0, 0);
        for (size_t i = 0; i < buffer.size(); i++) {
            result[i] = scale*(buffer[i] != static_cast<float>(noDataValue)) ? static_cast<float>(buffer[i]) : 0.0f;
        }
    } else if (dtype == GDT_UInt16) {
        std::vector<uint16_t> buffer(width * height);
        band->RasterIO(GF_Read, 0, 0, width, height, buffer.data(), width, height, GDT_UInt16, 0, 0);
        for (size_t i = 0; i < buffer.size(); i++) {
            result[i] = scale*(buffer[i] != static_cast<uint16_t>(noDataValue)) ? buffer[i] : 0.0f;
        }
    } else {
        std::cerr << "Unsupported data type: " << GDALGetDataTypeName(dtype) << std::endl;
    }

}


HeightMapTif::HeightMapTif(const std::string &filename, BoundingBox box){
    this->box = box;
    // **Open the dataset**
    GDALDataset* dataset = static_cast<GDALDataset*>(GDALOpen(filename.c_str(), GA_ReadOnly));
    if (!dataset) {
        std::cerr << "Failed to open " << filename << std::endl;
        return;
    }

    // Get the geotransform (affine transformation from pixel coordinates to geo-coordinates)
    double geoTransform[6];
    if (dataset->GetGeoTransform(geoTransform) != CE_None) {
        std::cerr << "Failed to get geotransform." << std::endl;
        return;
    }
    std::cout << "Successfully opened "+ filename << std::endl;

    // Get raster dimensions (width and height)
    width = dataset->GetRasterXSize();
    height = dataset->GetRasterYSize();

int scale = 100000000;
    // Process each band (assuming the dataset has 4 bands)
    processBand(dataset->GetRasterBand(1), 1, data1, scale); // Bands are 1-indexed in GDAL
    processBand(dataset->GetRasterBand(2), 2, data2, scale); // Bands are 1-indexed in GDAL
    processBand(dataset->GetRasterBand(3), 3, data3, scale); // Bands are 1-indexed in GDAL
    processBand(dataset->GetRasterBand(4), 4, data4, scale); // Bands are 1-indexed in GDAL

    data0.reserve(data1.size());

    for(int i=0 ; i < data1.size() ; ++i) {
        uint32_t heightValue = (data1[i]) | (data2[i] << 8) | (data3[i] << 16) |  (data4[i] << 24);
        // Convert to float
        float finalHeight;
        std::memcpy(&finalHeight, &heightValue, sizeof(float));
        data0.push_back(finalHeight);
    }        



    // **Close the dataset**
    GDALClose(dataset);

}

float HeightMapTif::getHeightAt(float x, float z) const {
    if(box.getMinX() <= x && x<= box.getMaxX() &&
    box.getMinZ() <= z && z<= box.getMaxZ()){
        glm::ivec2 absoluteCoordinates = glm::ivec2(
            int( width*(x-box.getMinX())/box.getLengthX()), 
            int( height*(z-box.getMinZ())/box.getLengthZ())
        );
        absoluteCoordinates = glm::clamp(absoluteCoordinates, glm::ivec2(0), glm::ivec2(width-1, height -1));
        int index = absoluteCoordinates.y * height + absoluteCoordinates.x;
        return data1[index]/1000.0;
        
    }
    return 0;
}
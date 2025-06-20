#include "math.hpp"



void processBand(GDALRasterBand* band, int bandIdx, std::vector<int16_t> &result) {
    if (band) {
        std::cout << "Processing Band " << (bandIdx) << std::endl;

    } else {
        std::cerr << "Band " << (bandIdx) << " is missing." << std::endl;
        return;
    }

    GDALDataType dtype = band->GetRasterDataType();

    // Retrieve the NoData value dynamically

     double noDataValue = band->GetNoDataValue(nullptr);  // Read NoData as double


    int width = band->GetXSize();
    int height = band->GetYSize();

    
    result.resize(width * height);

    size_t wh= width * height;

    if (dtype == GDT_Float32) {
        std::vector<float> buffer(wh);
        CPLErr err = band->RasterIO(GF_Read, 0, 0, width, height, buffer.data(), width, height, GDT_Float32, 0, 0);
        if (err != CE_None) {
            std::cerr << "Error reading raster data (Float32) for band!" << std::endl;
            return;
        }
        for (size_t i = 0; i < wh; i++) {
            result[i] = (buffer[i] != static_cast<float>(noDataValue)) ? buffer[i] : 0.0f;
        }
    } else if (dtype == GDT_UInt16) {
        std::vector<uint16_t> buffer(wh);
        CPLErr err = band->RasterIO(GF_Read, 0, 0, width, height, buffer.data(), width, height, GDT_UInt16, 0, 0);
        if (err != CE_None) {
            std::cerr << "Error reading raster data (UInt16) for band!" << std::endl;
            return;
        }
        for (size_t i = 0; i < wh; i++) {
            result[i] = (buffer[i] != static_cast<uint16_t>(noDataValue)) ? buffer[i] : 0.0f;
        }
        
    }  else if (dtype == GDT_Int16) {
        std::vector<int16_t> buffer(wh);
        CPLErr err = band->RasterIO(GF_Read, 0, 0, width, height, buffer.data(), width, height, GDT_Int16, 0, 0);
        if (err != CE_None) {
            std::cerr << "Error reading raster data (Int16) for band!" << std::endl;
            return;
        }
        for (size_t i = 0; i < wh; i++) {
            if (static_cast<double>(buffer[i]) != noDataValue) {  
                result[i] = buffer[i];  
            } else {
                std::cerr << "NoData" << std::endl;
                result[i] = 0.0f;  // NoData case
            }
        }
    }else {
        std::cerr << "Unsupported data type: " << GDALGetDataTypeName(dtype) << std::endl;
    }

}


HeightMapTif::HeightMapTif(const std::string &filename, BoundingBox box, int sizePerTile, float verticalScale, float verticalShift){
    this->box = box;
    this->sizePerTile = sizePerTile;
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
    std::cout << "Raster "+ std::to_string(width) << "x" << std::to_string(height) << std::endl;

    // Process band 1
    processBand(dataset->GetRasterBand(1), 1, data1); // Bands are 1-indexed in GDAL

   // Allocate the correct dimensions (height first)
    this->data = std::vector<std::vector<float>>(height, std::vector<float>(width)); 

    size_t wh = width * height;
    long sz=0;
    for(size_t i=0 ; i < wh ; ++i) {
        int y = i / width;  // Row index
        int x = i % width;  // Column index
    
        float floatValue = static_cast<float>(data1[i]) * verticalScale+verticalShift;
        this->data[y][x] = floatValue;
        ++sz;
        if (i < 8) {  // Debugging first few values
           // std::cout << "\th[" << y << "," << x << "] = " << floatValue << std::endl;
        }
        
    }        

    std::cout << "Calculated "+ std::to_string(sz) << " data[float]" << std::endl;


    // **Close the dataset**
    GDALClose(dataset);

}   

float HeightMapTif::getHeightAt(float x, float z) const {
    int ix = Math::clamp(int( width*(x-box.getMinX())/box.getLengthX()), 0, width-1);
    int iz = Math::clamp(int( height*(z-box.getMinZ())/box.getLengthZ()), 0, height-1);
    float result = data[iz][ix];
    return result;
}
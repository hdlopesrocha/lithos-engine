#include "math.hpp"



std::vector<float> processBand(GDALRasterBand* band) {
    // Get the NoData value for this band
    int noDataValue;
    band->GetNoDataValue(&noDataValue);
    
    // Get the width and height of the raster
    int width = band->GetXSize();
    int height = band->GetYSize();
    
    // Create a buffer to hold the pixel values
    uint16_t* buffer = (uint16_t*)CPLMalloc(width * sizeof(uint16_t));

    std::vector<float> result;
    // Read the data into the buffer
    for (int y = 0; y < height; y++) {
        CPLErr err = band->RasterIO(GF_Read, 0, y, width, 1, buffer, width, 1, GDT_UInt16, 0, 0);
        if (err != CE_None) {
            std::cerr << "Error reading data at line " << y << std::endl;
            break;
        }

        // Process the buffer
        for (int x = 0; x < width; x++) {
            uint16_t pixelValue = buffer[x];
            if (pixelValue != noDataValue) {
                // Do something with the pixel value (e.g., print or store it)
              //  std::cout << "Pixel [" << x << ", " << y << "] Value: " << pixelValue << std::endl;
            } else {
                std::cout << "Pixel [" << x << ", " << y << "] is NoData" << std::endl;
            }
            result.push_back(pixelValue);
        }
    }

    // Free the buffer memory
    CPLFree(buffer);
    return result;
}


HeightMapTif::HeightMapTif(const std::string & filename, BoundingBox box){
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

    // Get raster dimensions (width and height)
    width = dataset->GetRasterXSize();
    height = dataset->GetRasterYSize();


    // Process each band (assuming the dataset has 4 bands)
   // for (int bandIdx = 0; bandIdx < 4; bandIdx++) {
    int bandIdx = 0; 

        GDALRasterBand* band = dataset->GetRasterBand(bandIdx + 1); // Bands are 1-indexed in GDAL
        if (band) {
            std::cout << "Processing Band " << (bandIdx + 1) << std::endl;
            data = processBand(band);


        } else {
            std::cerr << "Band " << (bandIdx + 1) << " is missing." << std::endl;
        }
   // }


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
        int index = absoluteCoordinates.y * width + absoluteCoordinates.x;
        return data[index]/1000.0;
        
    }
    return 0;
}
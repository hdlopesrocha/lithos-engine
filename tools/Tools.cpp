#include "tools.hpp"

void ensureFolderExists(const std::string& folder) {
    if (!std::filesystem::exists(folder)) {
        std::filesystem::create_directories(folder);
    }
}
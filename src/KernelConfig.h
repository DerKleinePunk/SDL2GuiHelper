#pragma once
#include <string>

struct KernelConfig
{
    std::string AudioFileForClick;
    std::string AudioFileForLongClick;
    int lastMusikVolume;
    std::string mapDataPath;
    std::string mapStyle;
    std::vector<std::string> mapIconPaths;
};

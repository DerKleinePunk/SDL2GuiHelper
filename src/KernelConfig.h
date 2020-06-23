#pragma once
#include <string>
#include "../common/utils/Geopos.h"

struct KernelConfig
{
    std::string AudioFileForClick;
    std::string AudioFileForLongClick;
    int lastMusikVolume;
    std::string mapDataPath;
    std::string mapStyle;
    std::vector<std::string> mapIconPaths;
    std::string markerImageFile;
    utils::Geopos startMapPosition;
};

#pragma once
#include <string>
#include "../common/utils/Geopos.h"

struct KernelConfig
{
    KernelConfig(){
        lastMusikVolume = 128;//Value 0 - 128
        BackgroundScreen = white_color;
        ForegroundScreen = black_color;
    }
    std::string AudioFileForClick;
    std::string AudioFileForLongClick;
    int lastMusikVolume;
    std::string mapDataPath;
    std::string mapStyle;
    std::vector<std::string> mapIconPaths;
    std::string markerImageFile;
    utils::Geopos startMapPosition;
    SDL_Color BackgroundScreen;
    SDL_Color ForegroundScreen;
};

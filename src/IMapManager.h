#pragma once

#include <string>
#include <vector>

class IMapManager
{
private:
    /* data */
public:
    virtual ~IMapManager(){};

    virtual void SetScreenDpi(float screenDpi ) = 0;
    virtual int Init(std::string dataPath, std::string mapStyle, std::vector<std::string> mapIconPaths) = 0;
    virtual void Unregister() = 0;
    virtual void DeInit() = 0;
};


#pragma once

#include <string>
#include <vector>
#include <functional>

typedef std::function<void(unsigned char* mapPixels, int mapWidth, int mapHeight)> NewMapImageDelegate;
typedef std::function<void(const std::string& name, const int& maxSpeed, const int& currentSpeed)> NewStreetNameOrSpeedDelegate;

class IMapManager
{
private:
    /* data */
public:
    virtual ~IMapManager(){};

    virtual void SetScreenDpi(float screenDpi ) = 0;
    virtual int Init(std::string dataPath, std::string mapStyle, std::vector<std::string> mapIconPaths) = 0;
    virtual bool InitOk() const = 0;
    virtual void Unregister() = 0;
    virtual void DeInit() = 0;
    virtual void RegisterMe(int width, int height, NewMapImageDelegate callback, NewStreetNameOrSpeedDelegate callbackName) = 0;
    virtual void CenterMap(const double& lat,const double& lon, const double& compass, const double& currentSpeed) = 0;
    virtual void SetMarkerImageFile(const std::string& fileName) = 0;
    virtual void SetMarker(bool on) = 0;
    virtual void ZoomUp() = 0;
    virtual void ZoomDown() = 0;
    virtual void SetTargetPos(const double& lat,const double& lon) = 0;
};


#pragma once
#include <osmscout/Database.h>
#include <osmscout/GeoCoord.h>
#include <osmscout/LocationDescriptionService.h>
#include <osmscout/LocationService.h>
#include <osmscoutmapcairo/MapPainterCairo.h>
#include <osmscoutmap/MapService.h>
#include <osmscout/routing/SimpleRoutingService.h>
#include <osmscout/util/String.h>
#include "../../common/utils/osmsoutlogger.h"
#include "../../common/utils/waitingqueue.h"
#include "../IMapManager.h"
#include "MapObjects.h"


struct ThreadJobData {
    std::string whattodo;
    void* data1;
    void* data2;
};

class MapManager : public IMapManager
{
  private:
    float _screenDpi;
    std::string _dataPath;
    osmscout::DatabaseParameter _databaseParameter;
    osmscout::DatabaseRef _database;
    osmscout::MapServiceRef _mapService;
    osmscout::StyleConfigRef _styleConfig;
    osmscout::MapParameter _drawParameter;
    osmscout::AreaSearchParameter _searchParameter;
    osmscout::BreakerRef _breaker;
    osmscout::Magnification _magnification;
    waitingqueue<ThreadJobData*> _jobQueue;
    std::thread _worker;
    NewMapImageDelegate _callbackNewMapImage;
    NewStreetNameOrSpeedDelegate _callbackNewName;
    cairo_surface_t* _image_data_source;
    cairo_t* _cairoImage;
    cairo_surface_t* _image_data_marker;
    unsigned char* _mapPixels;
    osmscout::MapPainterCairo* _painter;
    bool _paintMarker;
    int _width;
    int _height;
    int _mapWidth;
    int _mapHeight;
    osmscout::MercatorProjection _projectionDraw;
    osmscout::MercatorProjection _projectionCalc;
    osmscout::MapData _data;
    std::list<osmscout::TileRef> _mapTiles;
    osmscout::GeoCoord _mapCenter;
    osmscout::GeoCoord _mapCenterJobStart;
    double _mapAngle;
    double _currentSpeed;
    std::string _markerImageFile;
    uint32_t _zoomValue;
    std::mutex _mutex;
    std::condition_variable _cv;
    MapObjects* _mapObjects;
    bool _initOk;

    int WorkerMain();
    void DrawMap();
    void LoadMapData();

  public:
    MapManager();
    ~MapManager();

    void SetScreenDpi(float screenDpi);
    int Init(std::string dataPath, std::string mapStyle, std::vector<std::string> mapIconPaths);
    bool InitOk() const;
    void RegisterMe(int width, int height, NewMapImageDelegate callback, NewStreetNameOrSpeedDelegate callbackName);
    void Unregister();
    void DeInit();
    void CenterMap(const double& lat, const double& lon, const double& compass, const double& currentSpeed);
    void SetMarkerImageFile(const std::string& fileName);
    void SetMarker(bool on);
    void ZoomUp();
    void ZoomDown();
    void SetTargetPos(const double& lat,const double& lon);
    void MoveMapPixel(double x, double y);
};

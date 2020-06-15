#pragma once
#include <osmscout/Database.h>
#include <osmscout/GeoCoord.h>
#include <osmscout/LocationDescriptionService.h>
#include <osmscout/LocationService.h>
#include <osmscout/MapPainterCairo.h>
#include <osmscout/MapService.h>
#include <osmscout/routing/SimpleRoutingService.h>
#include <osmscout/util/String.h>
#include "../../common/utils/osmsoutlogger.h"
#include "../../common/utils/waitingqueue.h"
#include "../IMapManager.h"

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

  public:
    MapManager();
    ~MapManager();

    void SetScreenDpi(float screenDpi);
    int Init(std::string dataPath, std::string mapStyle, std::vector<std::string> mapIconPaths);
    void Unregister();
    void DeInit();
};

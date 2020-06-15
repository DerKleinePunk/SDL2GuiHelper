#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "MapManager"
#endif

#include "MapManager.h"
#include "../../common/easylogging/easylogging++.h"
#include "../../common/utils/osmsoutlogger.h"

MapManager::MapManager()
{
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    _screenDpi = 96.0;
}

MapManager::~MapManager()
{
}

void MapManager::SetScreenDpi(float screenDpi )
{
    _screenDpi = screenDpi;
}

int MapManager::Init(std::string dataPath, std::string mapStyle, std::vector<std::string> mapIconPaths)
{
    osmscout::log.SetLogger(new utils::osmsoutlogger());
    std::list<std::string> paths;
    std::copy(mapIconPaths.begin(), mapIconPaths.end(), std::back_inserter( paths ) );
    _dataPath = dataPath;

    _database = std::make_shared<osmscout::Database>(_databaseParameter);
    if (!_database->Open(dataPath.c_str())) {
        LOG(ERROR) << "Cannot open database";
        return -1;
    }
    
    _mapService.reset(new osmscout::MapService(_database));
    _styleConfig.reset(new osmscout::StyleConfig(_database->GetTypeConfig()));
    if (!_styleConfig->Load(mapStyle)) {
        LOG(ERROR) << "Cannot open style";
        return -1;
    }

    _drawParameter.SetIconPaths(paths);
    _drawParameter.SetPatternPaths(paths);
    _drawParameter.SetFontSize(3.0);
    _drawParameter.SetDebugData(false);
    _drawParameter.SetDebugPerformance(true);

    return 0;
}

void MapManager::Unregister()
{

}

void MapManager::DeInit()
{

}
#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "MapManager"
#endif

#include "MapManager.h"
#include "../../common/easylogging/easylogging++.h"
#include "../../common/utils/osmsoutlogger.h"

int MapManager::WorkerMain()
{
    LOG(DEBUG) << "Map Worker Started";
    while(true) {
        try {
            auto jobInfo = _jobQueue.remove();
            if(jobInfo->whattodo == "Exit") {
                delete jobInfo;
                break;
            } else if(jobInfo->whattodo == "DrawMap") {
                delete jobInfo;
                osmscout::StopClock drawMapClock;
                DrawMap();
                drawMapClock.Stop();
                osmscout::StopClock searchMapDataClock;
                // SearchCurrentWayInfo();
                searchMapDataClock.Stop();
                LOG(INFO) << "Draw Map in " << drawMapClock.GetMilliseconds() << " ms";
                LOG(INFO) << "Search Map Data in " << searchMapDataClock.GetMilliseconds() << " ms";
            }
        } catch(std::exception& exp) {
            LOG(ERROR) << exp.what();
        }
    }

    _database->DumpStatistics();
    LOG(DEBUG) << "Map Worker Stopped";
    return 0;
}

void MapManager::DrawMap()
{
    LoadMapData();

    // memset(mapPixels_, 0xFF,  mapWidth_ * mapHeight_ * 4);

    if(_image_data_source == nullptr) {
        _image_data_source =
        cairo_image_surface_create_for_data(_mapPixels, CAIRO_FORMAT_ARGB32, _mapWidth, _mapHeight,
                                            cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, _mapWidth));
        _cairoImage = cairo_create(_image_data_source);
        if(_cairoImage == nullptr) {
            LOG(ERROR) << "cairoImage == nullptr";
            return;
        }
    }

    if(_painter->DrawMap(_projectionDraw, _drawParameter, _data, _cairoImage)) {
        LOG(DEBUG) << "Map drawed on surface";
        if(_paintMarker) {
            double x, y;
            _projectionDraw.GeoToPixel(_projectionDraw.GetCenter(), x, y);
            if(_image_data_marker == nullptr) {
                //Todo put to Config Imnage File Name
                _image_data_marker = cairo_image_surface_create_from_png("auto-day-icon.png");
                auto status = cairo_surface_status(_image_data_marker);
                if(status != CAIRO_STATUS_SUCCESS) {
                    LOG(ERROR) << "cairo load png " << cairo_status_to_string(status);
                    _paintMarker = false;
                }
            }
            if(_paintMarker) {
                auto imageWidth = cairo_image_surface_get_width(_image_data_marker);
                auto imageHeight = cairo_image_surface_get_height(_image_data_marker);
                cairo_set_source_surface(_cairoImage, _image_data_marker, x - (imageWidth / 2),
                                         y - (imageHeight / 2));
                cairo_paint(_cairoImage);
                LOG(DEBUG) << "drawed marker on surface";
            }
        }
        cairo_surface_flush(_image_data_source);
    }

    LOG(DEBUG) << "Map Draw Done";
    if(_callbackNewMapImage != nullptr) {
        _callbackNewMapImage(_mapPixels, _mapWidth, _mapHeight);
    }
}

void MapManager::LoadMapData() {
    _projectionDraw.Set(_mapCenter,
                    _mapAngle,
                    _magnification,
                    _screenDpi,
                    _mapWidth,
                    _mapHeight);

    if (_magnification.GetLevel()>=15) {
        _searchParameter.SetMaximumAreaLevel(6);
    }
    else {
        _searchParameter.SetMaximumAreaLevel(4);
    }

    _projectionDraw.SetLinearInterpolationUsage(_magnification.GetLevel() >= 10);
    _data.ClearDBData();
    
    _mapService->LookupTiles(_projectionDraw, _mapTiles);
    auto count = 0;
    for (auto it = _mapTiles.begin(); it != _mapTiles.end(); ++it) {
        if (!it->get()->IsComplete()) {
            count++;
        }
    }
    LOG(INFO) << "titles " << _mapTiles.size() << " size need load " << count;
    
    if(_mapService->LoadMissingTileData(_searchParameter, *_styleConfig, _mapTiles)) {
        _mapService->AddTileDataToMapData(_mapTiles, _data);
    }
}

MapManager::MapManager()
{
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    _screenDpi = 96.0;
    _mapPixels = nullptr;
    _callbackNewMapImage = nullptr;
    _cairoImage = nullptr;
    _image_data_source = nullptr;
    _image_data_marker = nullptr;
    _paintMarker = false;
    _mapAngle = 0;
    _width = 0;
    _height = 0;
    _mapCenter.Set(50.094, 8.49617);
}

MapManager::~MapManager()
{
    if(_mapPixels != nullptr) delete[] _mapPixels;
    if(_cairoImage != nullptr) {
        cairo_destroy(_cairoImage);
    }
    if(_image_data_source != nullptr) {
        cairo_surface_destroy(_image_data_source);
    }
    if(_image_data_marker != nullptr) {
        cairo_surface_destroy(_image_data_marker);
    }
}

void MapManager::SetScreenDpi(float screenDpi)
{
    _screenDpi = screenDpi;
}

int MapManager::Init(std::string dataPath, std::string mapStyle, std::vector<std::string> mapIconPaths)
{
    osmscout::log.SetLogger(new utils::osmsoutlogger());
    std::list<std::string> paths;
    std::copy(mapIconPaths.begin(), mapIconPaths.end(), std::back_inserter(paths));
    _dataPath = dataPath;

    _database = std::make_shared<osmscout::Database>(_databaseParameter);
    if(!_database->Open(dataPath.c_str())) {
        LOG(ERROR) << "Cannot open database";
        return -1;
    }

    _mapService.reset(new osmscout::MapService(_database));
    _styleConfig.reset(new osmscout::StyleConfig(_database->GetTypeConfig()));
    if(!_styleConfig->Load(mapStyle)) {
        LOG(ERROR) << "Cannot open style";
        return -1;
    }

    _drawParameter.SetIconPaths(paths);
    _drawParameter.SetPatternPaths(paths);
    _drawParameter.SetFontSize(3.0);
    _drawParameter.SetDebugData(false);
    _drawParameter.SetDebugPerformance(true);
    _drawParameter.SetRenderBackground(true); // we draw background before MapPainter
    _drawParameter.SetRenderUnknowns(false); // it is necessary to disable it with multiple databases
    _drawParameter.SetFontName("Inconsolata");
    _breaker = std::make_shared<osmscout::ThreadedBreaker>();
    _drawParameter.SetBreaker(_breaker);

    _magnification.SetLevel(osmscout::Magnification::magBlock);

    _searchParameter.SetUseMultithreading(true);

    /* Todo Routing
    osmscout::RouterParameter              routerParameter;
    routingService_ = std::make_shared<osmscout::SimpleRoutingService>(database_, routerParameter, routerFilenamebase_);

    if (!routingService_->Open()) {
         LOG(ERROR) << "Cannot open routing database";
         return -1;
    }

    //Todo Put Parameter to Config
    osmscout::TypeConfigRef             typeConfig=database_->GetTypeConfig();
    routingProfile_ = std::make_shared<osmscout::FastestPathRoutingProfile>(database_->GetTypeConfig());
    GetCarSpeedTable(carSpeedTable_);
    if(!routingProfile_->ParametrizeForCar(*typeConfig, carSpeedTable_, 130.0)) {
        LOG(WARNING) << "Missing Speedlimits for Types";
    }

    maxSpeedReader_ = new osmscout::MaxSpeedFeatureValueReader(*typeConfig);
    refFeatureReader_ =  new osmscout::RefFeatureValueReader(*typeConfig);*/

    _worker = std::thread(&MapManager::WorkerMain, this);

    return 0;
}

void MapManager::RegisterMe(int width, int height, NewMapImageDelegate callback, NewStreetNameOrSpeedDelegate callbackName) 
{
    _width = width;
    _height = height;

    _mapWidth = width * 2;
    _mapHeight = height * 2;
    
    _mapPixels = new unsigned char[_mapWidth * _mapHeight * 4];
    _callbackNewMapImage = callback;
    _callbackNewName = callbackName;
    
    auto mydata2 = new ThreadJobData();
    mydata2->whattodo = "DrawMap";
    mydata2->data1 = nullptr;
    mydata2->data2 = nullptr;

    _jobQueue.add(mydata2);
}

void MapManager::Unregister()
{
    _callbackNewMapImage = nullptr;
    _callbackNewName = nullptr;
}

void MapManager::DeInit()
{
    _callbackNewMapImage = nullptr;

    if(_worker.joinable()) {
        _jobQueue.clear();

        auto mydata2 = new ThreadJobData();
        mydata2->whattodo = "Exit";
        mydata2->data1 = nullptr;
        mydata2->data2 = nullptr;

        _jobQueue.add(mydata2);
        _worker.join();
    }
}
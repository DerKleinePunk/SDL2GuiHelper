#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "MapManager"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "MapManager.h"
#include "../../common/easylogging/easylogging++.h"
#include "../../common/utils/osmsoutlogger.h"
#include "../../common/exception/ArgumentException.h"
#include <chrono>
using namespace std::chrono_literals;


int MapManager::WorkerMain()
{
    _cv.notify_all();

    el::Helpers::setThreadName("MapManager Worker");
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

    if(_mapPixels == nullptr)  {
        LOG(DEBUG) << "no graphic buffer not paint the map";
        return;
    }

    if(_image_data_source == nullptr) {
        _image_data_source =
        cairo_image_surface_create_for_data(_mapPixels, CAIRO_FORMAT_ARGB32, _mapWidth, _mapHeight,
                                            cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, _mapWidth));
        _cairoImage = cairo_create(_image_data_source);
        _mapObjects = new MapObjects(_cairoImage);
        if(_cairoImage == nullptr) {
            LOG(ERROR) << "cairoImage == nullptr";
            return;
        }
    }

    if(_painter->DrawMap(_projectionDraw, _drawParameter, _data, _cairoImage)) {
        LOG(DEBUG) << "Map drawed on surface";
        double x, y;
        _projectionDraw.GeoToPixel(_projectionDraw.GetCenter(), x, y);
        if(_paintMarker) {
            if(_image_data_marker == nullptr) {
                _image_data_marker = cairo_image_surface_create_from_png(_markerImageFile.c_str());
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

        _mapObjects->DrawAll(_projectionDraw);
      
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
    _markerImageFile = "auto-day-icon.png";
    _zoomValue = 18;
    _mapWidth = 100;
    _mapHeight = 100;
    _mapObjects = nullptr;
}

MapManager::~MapManager()
{
    if(_mapPixels != nullptr) delete[] _mapPixels;

    if(_mapObjects != nullptr) {
        delete _mapObjects;
        _mapObjects = nullptr;
    }

    if(_cairoImage != nullptr) {
        cairo_destroy(_cairoImage);
    }

    if(_image_data_source != nullptr) {
        cairo_surface_destroy(_image_data_source);
        _image_data_source = nullptr;
    }

    if(_image_data_marker != nullptr) {
        cairo_surface_destroy(_image_data_marker);
         _image_data_marker = nullptr;
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

    _mapService = std::make_shared<osmscout::MapService>(_database);
    _styleConfig = std::make_shared<osmscout::StyleConfig>(_database->GetTypeConfig());
    if(!_styleConfig->Load(mapStyle)) {
        LOG(ERROR) << "Cannot open style";
        return -1;
    }

    _painter = new osmscout::MapPainterCairo(_styleConfig);

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

    _zoomValue = 18;
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

    std::unique_lock<std::mutex> startUpWait(_mutex);
    _worker = std::thread(&MapManager::WorkerMain, this);
    _cv.wait_for(startUpWait, 500ms);

    LOG(DEBUG) << "Init Done";

    return 0;
}

void MapManager::RegisterMe(int width, int height, NewMapImageDelegate callback, NewStreetNameOrSpeedDelegate callbackName) 
{
    if(_painter == nullptr) throw new ArgumentException("painter == nullptr");

    if(_mapPixels != nullptr) delete [] _mapPixels;
    
    if(_cairoImage != nullptr) {
        cairo_destroy(_cairoImage);
    }

    if(_image_data_source != nullptr) {
        cairo_surface_destroy(_image_data_source);
        _image_data_source = nullptr;
    }

    if(_image_data_marker != nullptr) {
        cairo_surface_destroy(_image_data_marker);
         _image_data_marker = nullptr;
    }

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

    if(_painter != nullptr) {
        delete _painter;
        _painter = nullptr;
    }

    if(_database->IsOpen()) {
        _database->Close();
    }
}

void MapManager::CenterMap(const double& lat,const double& lon, const double& compass, const double& currentSpeed) 
{
    LOG(DEBUG) << "CenterMap Lat " << std::to_string(lat) << " Lon " << std::to_string(lon);
    if (compass > -1) {
        _mapAngle = -compass * (2.0 * M_PI) / 360.0;
    }
                
    _mapCenter.Set(lat, lon);
    _currentSpeed = currentSpeed;
    //_paintMarker = true;
    
    _projectionCalc.Set(_mapCenter,
                    _mapAngle,
                    _magnification,
                    _screenDpi,
                    _mapWidth,
                    _mapHeight);
    
    double x, y;
    
    // New Pos every second NMEA Standard i think
    auto speedToDistance = _currentSpeed/3.6; //m/sec wenn speed km/h
    
    auto posIThink = _mapCenter.Add(osmscout::Bearing::Degrees(_mapAngle), osmscout::Distance::Of<osmscout::Meter>(speedToDistance));
    
    if(_projectionCalc.GeoToPixel(posIThink, x, y)) {
        LOG(DEBUG) << "Neue Pos ist auf Karte X " << x << " Y " << y << " Geo " << posIThink.GetDisplayText();
    }
       
    if(_jobQueue.size() == 0) {
        auto mydata2 = new ThreadJobData();
        mydata2->whattodo = "DrawMap";
        mydata2->data1 = nullptr;
        mydata2->data2 = nullptr;

        _jobQueue.add(mydata2);
    } else {
        LOG(WARNING) << "Redraw lost";
    }
}

void MapManager::SetMarkerImageFile(const std::string& fileName) {
    if(_image_data_marker != nullptr) {
        cairo_surface_destroy(_image_data_marker);
        _image_data_marker = nullptr;
    }

    _markerImageFile = fileName;
}

void MapManager::SetMarker(bool on)
{
    if(on && !_markerImageFile.empty()) {
        _paintMarker = true;
    } else {
        _paintMarker = false;
    }
}

void MapManager::ZoomUp()
{
    if(_magnification.GetLevel() + 1 <= 20) {
        _magnification++;
        _zoomValue = _magnification.GetLevel();

        auto mydata2 = new ThreadJobData();
        mydata2->whattodo = "DrawMap";
        mydata2->data1 = nullptr;
        mydata2->data2 = nullptr;

        _jobQueue.add(mydata2);
    }
}

/*
    static MagnificationLevel magWorld;     //  0
    static MagnificationLevel magContinent; //  4
    static MagnificationLevel magState;     //  5
    static MagnificationLevel magStateOver; //  6
    static MagnificationLevel magCounty;    //  7
    static MagnificationLevel magRegion;    //  8
    static MagnificationLevel magProximity; //  9
    static MagnificationLevel magCityOver;  // 10
    static MagnificationLevel magCity;      // 11
    static MagnificationLevel magSuburb;    // 12
    static MagnificationLevel magDetail;    // 13
    static MagnificationLevel magClose;     // 14
    static MagnificationLevel magCloser;    // 15
    static MagnificationLevel magVeryClose; // 16
    static MagnificationLevel magBlock;     // 18
    static MagnificationLevel magStreet;    // 19
    static MagnificationLevel magHouse;     // 20
*/
void MapManager::ZoomDown()
{
    if(_zoomValue - 1 >= 0) {
        _zoomValue--;
        switch (_zoomValue)
        {
            case 0:
                _magnification.SetLevel(osmscout::Magnification::magWorld);
                break;
            case 3:
                _magnification.SetLevel(osmscout::Magnification::magWorld);
                break;
            case 4:
                _magnification.SetLevel(osmscout::Magnification::magContinent);
                break;
            case 5:
                _magnification.SetLevel(osmscout::Magnification::magState);
                break;
            case 6:
                _magnification.SetLevel(osmscout::Magnification::magStateOver);
                break;
            case 7:
                _magnification.SetLevel(osmscout::Magnification::magCounty);
                break;
            case 8:
                _magnification.SetLevel(osmscout::Magnification::magRegion);
                break;
            case 9:
                _magnification.SetLevel(osmscout::Magnification::magProximity);
                break;
            case 10:
                _magnification.SetLevel(osmscout::Magnification::magCityOver);
                break;
            case 11:
                _magnification.SetLevel(osmscout::Magnification::magCity);
                break;
            case 12:
                _magnification.SetLevel(osmscout::Magnification::magSuburb);
                break;
            case 13:
                _magnification.SetLevel(osmscout::Magnification::magDetail);
                break;
            case 14:
                _magnification.SetLevel(osmscout::Magnification::magClose);
                break;
            case 15:
                _magnification.SetLevel(osmscout::Magnification::magCloser);
                break;
            case 16:
                _magnification.SetLevel(osmscout::Magnification::magVeryClose);
                break;
            case 18:
                _magnification.SetLevel(osmscout::Magnification::magBlock);
                break;
            case 19:
                _magnification.SetLevel(osmscout::Magnification::magStreet);
                break;
            case 20:
                _magnification.SetLevel(osmscout::Magnification::magHouse);
                break;
            default:
                break;
        }

        auto mydata2 = new ThreadJobData();
        mydata2->whattodo = "DrawMap";
        mydata2->data1 = nullptr;
        mydata2->data2 = nullptr;

        _jobQueue.add(mydata2);
    }
}

void MapManager::SetTargetPos(const double& lat,const double& lon)
{
    _mapObjects->SetTargetPos(lat, lon);
    
    if(_jobQueue.size() == 0) {
        auto mydata2 = new ThreadJobData();
        mydata2->whattodo = "DrawMap";
        mydata2->data1 = nullptr;
        mydata2->data2 = nullptr;

        _jobQueue.add(mydata2);
    } else {
        LOG(WARNING) << "Redraw lost";
    }
}
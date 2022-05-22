#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "MapDialog"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "MapDialog.h"
#include "../../src/gui/GUIElementManager.h"
#include "SampleApp.h"

MapDialog::MapDialog(GUIElement* parent, GUIElementManager* manager, SDLEventManager* eventManager)
{
    _parent = parent;
    _manager = manager;
    _autoMapUpdate = true;
    _eventManager = eventManager;
    _timerId = -1;
    _menuIsHide = false;
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
}

MapDialog::~MapDialog()
{
}

void MapDialog::Init()
{
    _mapView = new GUIMapview(GUIPoint(0, 0), GUISize(100, 100, sizeType::relative), "TheMap", white_color, black_color);
    //_mapView->RegisterOnFocus(std::bind(&SendSdsDialog::FocusChange, this, std::placeholders::_1));
    _manager->AddElement(_parent, _mapView);

    auto zoomUpButton = new GUITextButton(GUIPoint(939, 35), GUISize(80, 60), "zoomUpButton", own_firered_color, white_color);
    zoomUpButton->Text("+");
    zoomUpButton->RegisterOnClick([this](IGUIElement* sender) { 
        _mapView->ZoomUp();
    });
    _manager->AddElement(_parent, zoomUpButton);

    auto zoomDownButton = new GUITextButton(GUIPoint(939, 100), GUISize(80, 60), "zoomDownButton", own_firered_color, white_color);
    zoomDownButton->Text("-");
    zoomDownButton->RegisterOnClick([this](IGUIElement* sender) { 
        _mapView->ZoomDown();
    });
    _manager->AddElement(_parent, zoomDownButton);

    _mapView->SetMarker(_autoMapUpdate);
    _mapView->RegisterOnClick([this](IGUIElement* sender) {
        if(_menuIsHide) { 
            _eventManager->PushApplicationEvent(AppEvent::ShowMenu, nullptr, nullptr);
            if(_autoMapUpdate) {
                //Todo put Time to Config
                _timerId = _eventManager->CreateTimer(AppEvent::HideMenu, 15000, true);
            }
        }
    });
}

void MapDialog::Show()
{
    _parent->Visible();
    if(_autoMapUpdate) {
        //Todo put Time to Config
        _timerId = _eventManager->CreateTimer(AppEvent::HideMenu, 15000, true);
    }
}

void MapDialog::Hide()
{
    _parent->Invisible();
    if(_timerId != -1) {
        _eventManager->RemoveTimer(_timerId);
        _timerId = -1;
    }
    
}

void MapDialog::CenterMap(const double& lat,const double& lon, const double& compass, const double& currentSpeed) 
{
    if(_autoMapUpdate) _mapView->CenterMap(lat, lon, compass, currentSpeed);
}

void MapDialog::SetTargetPos(utils::Geopos position)
{
    _mapView->SetTargetPos(position.GetLat(), position.GetLon());
}

void MapDialog::HideMenu()
{
    GUIElement* element = _manager->GetElementByName("zoomUpButton");
    if(element != nullptr) {
        static_cast<GUITextButton*>(element)->Invisible();
    }
    element = _manager->GetElementByName("zoomDownButton");
    if(element != nullptr) {
        static_cast<GUITextButton*>(element)->Invisible();
    }
    _menuIsHide = true;
}

void MapDialog::ShowMenu()
{
    GUIElement* element = _manager->GetElementByName("zoomUpButton");
    if(element != nullptr) {
        static_cast<GUITextButton*>(element)->Visible();
    }
    element = _manager->GetElementByName("zoomDownButton");
    if(element != nullptr) {
        static_cast<GUITextButton*>(element)->Visible();
    }
     _menuIsHide = false;
}
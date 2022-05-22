#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "MusikPlayerDialog"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "MusikPlayerDialog.h"

#include "../../src/gui/GUIElementManager.h"
#include "SampleApp.h"

MusikPlayerDialog::MusikPlayerDialog(GUIElement* parent, GUIElementManager* manager, SDLEventManager* eventManager)
{
    _parent = parent;
    _manager = manager;
    _eventManager = eventManager;
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
}

MusikPlayerDialog::~MusikPlayerDialog()
{
}

void MusikPlayerDialog::Init()
{
    auto zoomDownButton = new GUITextButton(GUIPoint(100, 200), GUISize(300, 60), "playButton", own_firered_color, white_color);
    zoomDownButton->Text("Play Musik");
    zoomDownButton->RegisterOnClick([this](IGUIElement* sender) { PlayMusik(); });
    _manager->AddElement(_parent, zoomDownButton);

    /*_mapView = new GUIMapview(GUIPoint(0, 0), GUISize(100, 100, sizeType::relative), "TheMap", white_color,
    black_color); _manager->AddElement(_parent, _mapView);

    auto zoomUpButton = new GUITextButton(GUIPoint(939, 35), GUISize(80, 60), "zoomUpButton", own_firered_color,
    white_color); zoomUpButton->Text("+"); zoomUpButton->RegisterOnClick([this](IGUIElement* sender) {
        _mapView->ZoomUp();
    });
    _manager->AddElement(_parent, zoomUpButton);



    _mapView->SetMarker(_autoMapUpdate);
    _mapView->RegisterOnClick([this](IGUIElement* sender) {
        if(_menuIsHide) {
            _eventManager->PushApplicationEvent(AppEvent::ShowMenu, nullptr, nullptr);
            if(_autoMapUpdate) {
                //Todo put Time to Config
                _timerId = _eventManager->CreateTimer(AppEvent::HideMenu, 15000, true);
            }
        }
    });*/
}

void MusikPlayerDialog::Show()
{
    _parent->Visible();
}

void MusikPlayerDialog::Hide()
{
    _parent->Invisible();
}

void MusikPlayerDialog::PlayMusik()
{
    _currentMusikFile = "test.mp3";
    _eventManager->PushKernelEvent(KernelEvent::MusikStartStream, &_currentMusikFile, nullptr);
}
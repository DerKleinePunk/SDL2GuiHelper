#pragma once
#include "../../src/Elements.h"
#include "../../common/utils/Geopos.h"

class MapDialog
{
private:
    GUIElement* _parent;
    GUIElementManager* _manager;
    GUIMapview* _mapView;
    SDLEventManager* _eventManager;
    SDL_TimerID _timerId;
    bool _autoMapUpdate;
    bool _menuIsHide;
public:
    MapDialog(GUIElement* parent, GUIElementManager* manager, SDLEventManager* eventManager);
    ~MapDialog();

    void Init();
    void Show();
    void Hide();

    void CenterMap(const double& lat,const double& lon, const double& compass, const double& currentSpeed);
    void SetTargetPos(utils::Geopos position);

    void HideMenu();
    void ShowMenu();

};

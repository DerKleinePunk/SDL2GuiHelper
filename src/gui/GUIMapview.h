#pragma once
#include <SDL_ttf.h>
#include "GUIOnClickDecorator.h"

struct GUIEvent;
class GUITexture;
class MapManager;

class GUIMapview : public GUIElement, public GUIOnClickDecorator{
    el::Logger* logger_;
    bool initOk_;
    SDL_Color textcolor_{};
    TTF_Font* font_;
    SDL_mutex*          mapMemLock_;
    unsigned char*      mapPixels_;
    GUITexture*         mapTexture_;
    Uint32              mapEvent_{};
    SDL_Point           viewDeltaNow_{};
    SDL_Point           startPoint_{};
    SDL_Point           viewDeltaAtJob_{};
    GUITexture*         streetImageTexture_;
    GUITexture*         headTexture_;
    GUITexture*         statusbarTexture_;
    std::string         currentStreetName_;
    int                 currentMaxSpeed_{};
    int                 currentSpeed_{};
    bool                renderJobRun_;
    bool                _buttonIsDown;
      
    void RenderMap();
    void InitMap();
    void HandleMapEvent(GUIEvent& event);
    int MapThreadMain();
    void NewMapPixel(unsigned char* mapPixels, int mapWidth, int mapHeight);
    void NewMapNameOrSpeed(const std::string& name, const int& maxSpeed, const int& currentSpeed);

public:
    GUIMapview(GUIPoint position, const GUISize& size, const std::string& name, SDL_Color background, SDL_Color textcolor);

    void Init() override;
    void Draw() override;
    void HandleEvent(GUIEvent& event) override;
    void UpdateAnimation() override;
    void Close() override;
       
    void CenterMap(const double& lat,const double& lon, const double& compass, const double& currentSpeed);
    void SetMarker(bool on);
    void ZoomUp();
    void ZoomDown();
    void SetTargetPos(const double& lat,const double& lon);

    void ButtonDownUpdate(Uint8 button, const GUIPoint& point) override;
	void ButtonUpUpdate(Uint8 button, const GUIPoint& point) override;
    void ButtonMoveUpdate(Uint8 button, const GUIPoint& point) override;
};

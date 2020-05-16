#pragma once

#include <list>
#include <vector>
#include <string>
#include <SDL_ttf.h>

struct GUIPoint;
struct GUISize;
struct GUIEvent;
class GUITexture;

const std::string KEYCAP_BACKSPACE = "\u2190";
const std::string KEYCAP_SHIFT     = "\u2191";
const std::string KEYCAP_SYMBOLS   = "SYM";
const std::string KEYCAP_ABC       = "abc";
const std::string KEYCAP_SPACE     = " ";
const std::string KEYCAP_RETURN = "\n";

struct touchArea {
  std::string keyChar;
  GUIRect rect;
};

struct KeyboardLayer{
  GUITexture *texture;
  std::list<std::string> row1;
  std::list<std::string> row2;
  std::list<std::string> row3;
  std::list<std::string> row4;
  std::vector<touchArea> keyList;
  int layerNum;
};

class GUIKeyboard : public GUIElement
{
    RTTI_DERIVED(GUIKeyboard);
    std::list<KeyboardLayer> keyboard_;
    TTF_Font* font_;
    int currentLayer_;
    std::string lastKeyDown_;
    void LoadKeymap();
    void DrawLayer(KeyboardLayer* layer);
    void DrawRow(std::vector<touchArea> *keyList, int x, int y, int width, int height, std::list<std::string> *keys, int padding, TTF_Font *font);
    void DrawKey(std::vector<touchArea> *keyList, int x, int y, int width, int height, const char *cap, const std::string* key,int padding, TTF_Font *font);
    std::string GetKeyFromPoint(const GUIPoint& point);
    void ButtonDown(Uint8 button, Uint8 clicks, const GUIPoint& point);
    void ButtonUp(Uint8 button, Uint8 clicks, const GUIPoint& point);
public:
    GUIKeyboard(GUIPoint position, GUISize size, SDL_Color background, const std::string& name);
    ~GUIKeyboard();

    void Init() override;
    void Draw() override;
    void HandleEvent(GUIEvent& event) override;
    void UpdateAnimation() override;
    void Close() override;
};


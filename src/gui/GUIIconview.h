#pragma once

struct GUIEvent;
class GUITexture;

class GUIIconview : public GUIElement {
    el::Logger* logger_;
	GUITexture* imageTexture_;
public:
	GUIIconview(GUIPoint position, GUISize size, const std::string& name, SDL_Color background);

    void Init() override;
    void Draw() override;
    void HandleEvent(GUIEvent& event) override;
    void UpdateAnimation() override;
    void Close() override;

	void SetCurrentIcon(const std::string& fileName);
};

#pragma once

#include "IGUIElement.h"

struct GUIEvent;
class SDLEventManager;

class GUIElementDecorator : public IGUIElement
{
protected:
	IGUIElement* wrapper_;
public:
	explicit GUIElementDecorator(IGUIElement* wrapper);
	~GUIElementDecorator();

	SDLEventManager* EventManager() override;
	void Init() override;
	void Draw() override;
	void HandleEvent(GUIEvent& event) override;
	void UpdateAnimation() override;
	void Close() override;
};

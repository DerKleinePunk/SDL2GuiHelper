#pragma once
#include "GUI.h"

struct GUIEvent;
class SDLEventManager;

class IGUIElement
{
	friend class GUIOnClickDecorator;

protected:
	ButtonDelegate buttonDownEvent_;
	ButtonDelegate buttonUpEvent_;
	MouseDelegate mouseMoveEvent_;
	MousePositionDelegate mouseLeaveEvent_;

public:
	IGUIElement();
	virtual ~IGUIElement()
	{
	}

	//The Interface
	virtual SDLEventManager* EventManager() = 0;
	virtual void Init() = 0;
	virtual void Draw() = 0;
	virtual void HandleEvent(GUIEvent& event) = 0;
	virtual void UpdateAnimation() = 0;
	virtual void Close() = 0;
};

inline IGUIElement::IGUIElement():
	buttonDownEvent_(nullptr),
	buttonUpEvent_(nullptr),
	mouseMoveEvent_(nullptr),
	mouseLeaveEvent_(nullptr)
{
}

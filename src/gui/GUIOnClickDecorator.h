#pragma once
#include "GUIElementDecorator.h"

typedef std::function<void(IGUIElement* sender)> ClickDelegate;

class GUIOnClickDecorator : public GUIElementDecorator
{
	ClickDelegate OnClick_;
	ClickDelegate OnLongClick_;
	Uint32 lastLeftButtonDow_;
	Uint8 _lastButton;

	void ButtonDown(Uint8 button, Uint8 clicks, const GUIPoint& point);
	void ButtonUp(Uint8 button, Uint8 clicks, const GUIPoint& point);
	void MouseLeave(const GUIPoint& point);
	void MouseMoveEvent(Uint8 button, const GUIPoint& point);

public:
	GUIOnClickDecorator(IGUIElement* wrapper);

	void RegisterOnClick(ClickDelegate OnClick);
	void RegisterOnLongClick(ClickDelegate OnClick);

	virtual void ButtonDownUpdate(Uint8 button, const GUIPoint& point) = 0;
	virtual void ButtonUpUpdate(Uint8 button, const GUIPoint& point) = 0;
	virtual void ButtonMoveUpdate(Uint8 button, const GUIPoint& point) = 0;
};


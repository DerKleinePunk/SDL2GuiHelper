#pragma once
#include "GUIElementDecorator.h"

typedef std::function<void(IGUIElement* sender)> ClickDelegate;

class GUIOnClickDecorator : public GUIElementDecorator
{
	ClickDelegate OnClick_;
	ClickDelegate OnLongClick_;
	Uint32 lastLeftButtonDow_;

	void ButtonDown(Uint8 button, Uint8 clicks);
	void ButtonUp(Uint8 button, Uint8 clicks);

public:
	GUIOnClickDecorator(IGUIElement* wrapper);

	void RegisterOnClick(ClickDelegate OnClick);
	void RegisterOnLongClick(ClickDelegate OnClick);
};


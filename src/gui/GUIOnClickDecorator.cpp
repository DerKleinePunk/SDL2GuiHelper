#include "GUI.h"
#include "IGUIElement.h"
#include "GUIElement.h"
#include "GUIOnClickDecorator.h"
#include "../SDLEventManager.h"
#include <AppEvents.h>

void GUIOnClickDecorator::ButtonDown(Uint8 button, Uint8 clicks)
{
    UNUSED(clicks);
	_lastButton = button;
	ButtonDownUpdate(button);

	if (button == SDL_BUTTON_LEFT) {
		lastLeftButtonDow_ = SDL_GetTicks();
	}
}

void GUIOnClickDecorator::ButtonUp(Uint8 button, Uint8 clicks)
{
    UNUSED(clicks);
	ButtonUpUpdate(button);

	if (button == SDL_BUTTON_LEFT) {
		auto now = SDL_GetTicks();
		if (lastLeftButtonDow_ != 0) {
			if (SDL_TICKS_PASSED(now, lastLeftButtonDow_ + long_click_time)) {
				if (OnLongClick_) {
					EventManager()->PushApplicationEvent(AppEvent::LongClick, nullptr, nullptr);
                    OnLongClick_(wrapper_);
				}
			}
			else {
				if (OnClick_) {
					EventManager()->PushApplicationEvent(AppEvent::Click, nullptr, nullptr);
					OnClick_(wrapper_);
				}
			}
			lastLeftButtonDow_ = 0;
		}
	}
}

void GUIOnClickDecorator::MouseLeave(const GUIPoint& point)
{
	UNUSED(point);
	ButtonUpUpdate(_lastButton);
}

GUIOnClickDecorator::GUIOnClickDecorator(IGUIElement* wrapper)
	: GUIElementDecorator(wrapper),_lastButton(SDL_BUTTON_LEFT)
{
	wrapper->buttonDownEvent_ = std::bind(&GUIOnClickDecorator::ButtonDown, this, std::placeholders::_1, std::placeholders::_2);
	wrapper->buttonUpEvent_ = std::bind(&GUIOnClickDecorator::ButtonUp, this, std::placeholders::_1, std::placeholders::_2);
	wrapper->mouseLeaveEvent_ = std::bind(&GUIOnClickDecorator::MouseLeave, this, std::placeholders::_1);
}

void GUIOnClickDecorator::RegisterOnClick(ClickDelegate OnClick)
{
	OnClick_ = OnClick;
}

void GUIOnClickDecorator::RegisterOnLongClick(ClickDelegate OnClick)
{
	OnLongClick_ = OnClick;
}

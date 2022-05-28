#include "GUI.h"
#include "IGUIElement.h"
#include "GUIElement.h"
#include "GUIOnClickDecorator.h"
#include "../SDLEventManager.h"
#include <AppEvents.h>

void GUIOnClickDecorator::ButtonDown(Uint8 button, Uint8 clicks, const GUIPoint& point)
{
    UNUSED(clicks);
	_lastButton = button;
	ButtonDownUpdate(button, point);

	if (button == SDL_BUTTON_LEFT) {
		lastLeftButtonDow_ = SDL_GetTicks();
	}
}

void GUIOnClickDecorator::ButtonUp(Uint8 button, Uint8 clicks, const GUIPoint& point)
{
    UNUSED(clicks);
	ButtonUpUpdate(button, point);

	if (button == SDL_BUTTON_LEFT) {
		auto now = SDL_GetTicks();
		if (lastLeftButtonDow_ != 0) {
			if (SDL_TICKS_PASSED(now, lastLeftButtonDow_ + long_click_time)) {
				if (OnLongClick_) {
					EventManager()->PushApplicationEvent(AppEvent::LongClick, (void*)&point, nullptr);
                    OnLongClick_(wrapper_);
				}
			}
			else {
				if (OnClick_) {
					EventManager()->PushApplicationEvent(AppEvent::Click, (void*)&point, nullptr);
					OnClick_(wrapper_);
				}
			}
			lastLeftButtonDow_ = 0;
		}
	}
}

void GUIOnClickDecorator::MouseLeave(const GUIPoint& point)
{
	ButtonUpUpdate(_lastButton, point);
}

void GUIOnClickDecorator::MouseMoveEvent(Uint8 button, const GUIPoint& point)
{
	ButtonMoveUpdate(button, point);
}

GUIOnClickDecorator::GUIOnClickDecorator(IGUIElement* wrapper)
	: GUIElementDecorator(wrapper),_lastButton(SDL_BUTTON_LEFT)
{
	wrapper->buttonDownEvent_ = std::bind(&GUIOnClickDecorator::ButtonDown, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	wrapper->buttonUpEvent_ = std::bind(&GUIOnClickDecorator::ButtonUp, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	wrapper->mouseLeaveEvent_ = std::bind(&GUIOnClickDecorator::MouseLeave, this, std::placeholders::_1);
	wrapper->mouseMoveEvent_ = std::bind(&GUIOnClickDecorator::MouseMoveEvent, this, std::placeholders::_1, std::placeholders::_2);
}

void GUIOnClickDecorator::RegisterOnClick(ClickDelegate OnClick)
{
	OnClick_ = OnClick;
}

void GUIOnClickDecorator::RegisterOnLongClick(ClickDelegate OnClick)
{
	OnLongClick_ = OnClick;
}

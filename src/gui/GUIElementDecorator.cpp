#include "IGUIElement.h"
#include "GUIElementDecorator.h"
#include "../SDLEventManager.h"

GUIElementDecorator::GUIElementDecorator(IGUIElement* wrapper)
{
	wrapper_ = wrapper;
}

GUIElementDecorator::~GUIElementDecorator()
{
	wrapper_ = nullptr;
}

SDLEventManager* GUIElementDecorator::EventManager()
{
	return wrapper_->EventManager();
}

void GUIElementDecorator::Init()
{
	wrapper_->Init();
}

void GUIElementDecorator::Draw()
{
	wrapper_->Draw();
}

void GUIElementDecorator::HandleEvent(GUIEvent& event)
{
	wrapper_->HandleEvent(event);
}

void GUIElementDecorator::UpdateAnimation()
{
	wrapper_->UpdateAnimation();
}

void GUIElementDecorator::Close()
{
	wrapper_->Close();
}

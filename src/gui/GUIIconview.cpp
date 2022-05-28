#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "GUIIconview"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif
#include "../../common/easylogging/easylogging++.h"

#include "GUI.h"
#include "GUIElement.h"
#include "GUIOnClickDecorator.h"
#include "GUIIconview.h"
#include "GUITexture.h"
#include "GUIRenderer.h"
#include "GUIImageManager.h"


GUIIconview::GUIIconview(const GUIPoint position, const GUISize size, const std::string& name, const SDL_Color background) :
	GUIElement(position, size, name),
	GUIOnClickDecorator(static_cast<GUIElement*>(this)),
	imageTexture_(nullptr) {
	logger_ = el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
	backgroundColor_ = background;
}

void GUIIconview::Init() {
    //Things after Control is Created
}

void GUIIconview::Draw() {
	auto centerX = Size().width / 2;
	auto centerY = Size().height / 2;

	centerX--;
	centerY--;

	if (imageTexture_ != nullptr) {
		const auto size = imageTexture_->Size();
		const auto x = centerX - size.width / 2;
		const auto y = centerY - size.height / 2;
		renderer_->RenderCopy(imageTexture_, GUIPoint(x, y));
	}

    needRedraw_ = false;
}

void GUIIconview::HandleEvent(GUIEvent& event) {
	UNUSED(event);
}

void GUIIconview::UpdateAnimation() {
}

void GUIIconview::Close() {
    
}

void GUIIconview::SetCurrentIcon(const std::string& fileName) {
	imageTexture_ = imageManager_->GetImage(fileName);
	SetRedraw();
}

void GUIIconview::ButtonDownUpdate(Uint8 button, const GUIPoint& point)
{

}

void GUIIconview::ButtonUpUpdate(Uint8 button, const GUIPoint& point)
{

}

void GUIIconview::ButtonMoveUpdate(Uint8 button, const GUIPoint& point)
{
    UNUSED(button);
}
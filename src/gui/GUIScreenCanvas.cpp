#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "GUIScreenCanvas"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif
#include "../../common/easylogging/easylogging++.h"

#include "GUI.h"
#include "GUIElement.h"
#include "GUIRenderer.h"
#include "GUIScreenCanvas.h"
#include "GUITexture.h"
#include "GUIImageManager.h"

GUIScreenCanvas::GUIScreenCanvas(GUISize size, const std::string& backgroundImage):
	GUIElement(GUIPoint(0, 0), size, "ScreenCanvas"), 
	imageTexture_(nullptr) {
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
	backgroundColor_ = white_color;
	_backgroundImage = backgroundImage;
}

void GUIScreenCanvas::Resize(GUISize size)
{
	size_ = size;
	renderer_->ResizeTexture(Texture(), size_);
	needRedraw_ = true;
}

void GUIScreenCanvas::Init() {
    LOG(DEBUG) << "GUIScreenCanvas Init";
	if(_backgroundImage.size() > 0) {
		imageTexture_ = imageManager_->GetImage(_backgroundImage);
	}
}

void GUIScreenCanvas::Draw()
{
	if (imageTexture_ != nullptr) {
		auto centerX = Size().width / 2;
		auto centerY = Size().height / 2;
		auto size = imageTexture_->Size();
		auto x = centerX - size.width / 2;
		auto y = centerY - size.height / 2;
		renderer_->RenderCopy(imageTexture_, GUIPoint(x, y));
	}
	needRedraw_ = false;
}

void GUIScreenCanvas::HandleEvent(GUIEvent& event)
{
    UNUSED(event);
}

void GUIScreenCanvas::UpdateAnimation()
{
}

void GUIScreenCanvas::Close()
{
}

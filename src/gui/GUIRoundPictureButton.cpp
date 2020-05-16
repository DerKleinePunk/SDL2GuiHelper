#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "GUIRoundPictureButton"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif
#include "../../common/easylogging/easylogging++.h"
#include "GUI.h"
#include "IGUIElement.h"
#include "GUIElement.h"
#include "GUIOnClickDecorator.h"
#include "GUIRoundPictureButton.h"
#include "GUIImageManager.h"
#include "GUIRenderer.h"
#include "GUITexture.h"

GUIRoundPictureButton::GUIRoundPictureButton(GUIPoint position, GUISize size, const std::string& name, SDL_Color background, SDL_Color textcolor):
	GUIElement(position, size, name),
	GUIOnClickDecorator(static_cast<GUIElement*>(this)),
	imageTexture_(nullptr),
	imageTextureDisable_(nullptr), 
    imageTextureBackground_(nullptr),
	imageSelected_(nullptr),
	imageSelectedBackground_(nullptr) {
	logger_ = el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
	backgroundColor_ = red_color;
	backgroundColor_.a = 0x00;
	backgroundColorCircle_ = background;
	foregroundColor_ = textcolor;

	centerX_ = -1;
	centerY_ = -1;

	hasBorder_ = true;
	pictureSize_ = GUISize(-1, -1);
}

void GUIRoundPictureButton::Image(const std::string fileName) {
    imageTexture_ = imageManager_->GetImage(fileName);
	SetRedraw();
}

void GUIRoundPictureButton::ImageDisable(const std::string fileName) {
	imageTextureDisable_ = imageManager_->GetImage(fileName);
	SetRedraw();
}

void GUIRoundPictureButton::ImageBackground(const std::string fileName) {
	imageTextureBackground_ = imageManager_->GetImage(fileName);
	SetRedraw();
}

void GUIRoundPictureButton::ImageSelected(const std::string fileName) {
	imageSelected_ = imageManager_->GetImage(fileName);
	SetRedraw();
}

void GUIRoundPictureButton::ImageSelectedBackground(const std::string fileName) {
	imageSelectedBackground_ = imageManager_->GetImage(fileName);
	SetRedraw();
}

void GUIRoundPictureButton::Border(bool on) {
	hasBorder_ = on;
}

void GUIRoundPictureButton::PictureSize(const GUISize& size) {
	pictureSize_ = size;
	needRedraw_ = true;
}

void GUIRoundPictureButton::Init()
{
    LOG(DEBUG) << "hidden on init " << (hidden_ == true ? "true" : "false");
	Texture()->SetTextureAlphaMod(0xFF);
}

void GUIRoundPictureButton::Draw() {
	centerX_ = Size().width / 2;
	centerY_ = Size().height / 2;
	const auto with = centerX_ - 1;
	const auto height = centerY_ - 1;
	centerX_--;
	centerY_--;
	
	if (imageTextureBackground_ == nullptr) {
		renderer_->DrawMidpointEllipse(centerX_, centerY_, with, height, backgroundColorCircle_, true, 0);
	} else {
		if(selected_ && imageSelectedBackground_) {
			const auto size = imageSelectedBackground_->Size();
			const auto x = centerX_ - size.width / 2;
			const auto y = centerY_ - size.height / 2;
			renderer_->RenderCopy(imageSelectedBackground_, GUIPoint(x, y));
		}
		else {
			const auto size = imageTextureBackground_->Size();
			const auto x = centerX_ - size.width / 2;
			const auto y = centerY_ - size.height / 2;
			renderer_->RenderCopy(imageTextureBackground_, GUIPoint(x, y));
		}
	}
	
	if (!enabled_ && imageTextureDisable_ != nullptr) {
		if (pictureSize_.width > 0) {
			const auto x = centerX_ - pictureSize_.width / 2;
			const auto y = centerY_ - pictureSize_.height / 2;
			renderer_->RenderCopy(imageTextureDisable_, GUIRect(x, y, pictureSize_.width, pictureSize_.height));
		}
		else {
			const auto size = imageTextureDisable_->Size();
			const auto x = centerX_ - size.width / 2;
			const auto y = centerY_ - size.height / 2;
			renderer_->RenderCopy(imageTextureDisable_, GUIPoint(x, y));
		}
	}
	else
	{
		if(selected_ && imageSelected_ && imageSelected_->IsValid()) {
			if (pictureSize_.width > 0) {
				const auto x = centerX_ - pictureSize_.width / 2;
				const auto y = centerY_ - pictureSize_.height / 2;
				renderer_->RenderCopy(imageSelected_, GUIRect(x, y, pictureSize_.width, pictureSize_.height));
			}
			else {
				const auto size = imageSelected_->Size();
				const auto x = centerX_ - size.width / 2;
				const auto y = centerY_ - size.height / 2;
				renderer_->RenderCopy(imageSelected_, GUIPoint(x, y));
			}
		}
		else {
			if (imageTexture_ && imageTexture_->IsValid()) {
				if (pictureSize_.width > 0) {
					const auto x = centerX_ - pictureSize_.width / 2;
					const auto y = centerY_ - pictureSize_.height / 2;
					renderer_->RenderCopy(imageTexture_, GUIRect(x, y, pictureSize_.width, pictureSize_.height));
				}
				else {
					const auto size = imageTexture_->Size();
					const auto x = centerX_ - size.width / 2;
					const auto y = centerY_ - size.height / 2;
					renderer_->RenderCopy(imageTexture_, GUIPoint(x, y));
				}
			}
		}
	}
	
	if(hasBorder_){
		renderer_->DrawMidpointEllipse(centerX_, centerY_, with - 1, height - 1, foregroundColor_, false, 1);
		//renderer_->DrawMidpointEllipse(centerX_, centerY_, with, height, foregroundColor_, false, 1);
	}

	if(!enabled_ && imageTextureDisable_ == nullptr) {
		renderer_->DrawMidpointEllipse(centerX_, centerY_, with, height, black_t_color, true, 0);
	}

	needRedraw_ = false;
}

void GUIRoundPictureButton::HandleEvent(GUIEvent& event)
{
    UNUSED(event);
}

void GUIRoundPictureButton::UpdateAnimation()
{
}

void GUIRoundPictureButton::Close() {
}

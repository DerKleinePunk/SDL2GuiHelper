#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "GUIMediaDisplay"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif
#include "../../common/easylogging/easylogging++.h"

#include "GUI.h"
#include "GUIElement.h"
#include "GUIOnClickDecorator.h"
#include "GUIMediaDisplay.h"
#include "GUIFontManager.h"
#include "GUIRenderer.h"
#include "GUITexture.h"

#ifdef ENABLEAUDIOMANAGER
#include "../sound/IAudioManager.h"
#endif

void GUIMediaDisplay::GetFont() {
	if(!smallFont_) {
		font_ = fontManager_->GetDefaultFont(fontHeight_);
	} else {
		font_ = fontManager_->GetDefaultSmallFont(fontHeight_);
	}
	
}

GUIMediaDisplay::GUIMediaDisplay(const GUIPoint position, const GUISize size, const std::string& name, const SDL_Color background, const SDL_Color textcolor):
	GUIMediaDisplay(position, size, name, background, textcolor, false){
}

GUIMediaDisplay::GUIMediaDisplay(const GUIPoint position, const GUISize size, const std::string& name, const SDL_Color background, const SDL_Color textcolor, bool smallFont) :
	GUIElement(position, size, name),
	GUIOnClickDecorator(static_cast<GUIElement*>(this)),
	smallFont_(smallFont),
	font_(nullptr),
	textureText_(nullptr), 
	textureMediadata_(nullptr),
	audioManager_(nullptr) {
	backgroundColor_ = background;
	foregroundColor_ = textcolor;
	selectedColor_ = own_red_color;
	angle_ = 0;
	fontHeight_ = -1;
	textAnchor_ = AnchorFlags::Right & AnchorFlags::Left;
	text_ = "Keine Media Daten";
	running_ = false;
}


void GUIMediaDisplay::FontHeight(const int fontHeight) {
	fontHeight_ = fontHeight;
	if (font_ != nullptr) {
		GetFont();
		RenderText();
	}
	needRedraw_ = true;
}

void GUIMediaDisplay::TextAnchor(const AnchorFlags flags) {
	textAnchor_ = flags;
}

void GUIMediaDisplay::StreamStatus(const bool running) {
	running_ = running;
	if(!running) {
		SetRedraw();
	}
}

void GUIMediaDisplay::Init() {
	//Things after Control is Created
	if(fontHeight_ == -1) {
		fontHeight_ = Size().height;
	}
	GetFont();
	RenderText();

	auto audiodataSize = Size();
	audiodataSize.height -= 30;

	textureMediadata_ = renderer_->CreateTexture(audiodataSize);
	textureMediadata_->SetBlendMode(blendMode::blend);
}

void GUIMediaDisplay::SetAudioManager(IAudioManager* manager) {
	audioManager_ = manager;
}

void GUIMediaDisplay::RenderText() {
	if (textureText_ != nullptr) delete textureText_;
	if(text_.find("\n") != std::string::npos) {
		if (selected_) {
			textureText_ = renderer_->RenderTextBlendedWrapped(font_, text_, selectedColor_, Size().width);
		}
		else {
			textureText_ = renderer_->RenderTextBlendedWrapped(font_, text_, foregroundColor_, Size().width);
		}
		return;
	}
	if(selected_) {
		textureText_ = renderer_->RenderTextBlended(font_, text_, selectedColor_);
	} else {
		textureText_ = renderer_->RenderTextBlended(font_, text_, foregroundColor_);
	}
	SetRedraw();
}

//http://stackoverflow.com/questions/3229391/sdl-sdl-ttf-transparent-blended-text
void GUIMediaDisplay::Draw() {

	if(running_ && textureMediadata_ != nullptr) {
		renderer_->RenderCopy(textureMediadata_, GUIPoint(0, 0));
	}

	if (font_ == nullptr) {
		GetFont();
	}
	if (text_.size() > 0 && textureText_ == nullptr) {
		RenderText();
	}
	
	if(!running_) {
		if (fontHeight_ > -1 && textureText_ != nullptr) {
			if (angle_ == 0) {
				if (textAnchor_ == AnchorFlags::Left) {
					renderer_->RenderCopy(textureText_, GUIPoint(Size().width - textureText_->Size().width, 0));
				} else if (textAnchor_ == (AnchorFlags::Left & AnchorFlags::Right)) {
					renderer_->RenderCopy(textureText_, GUIPoint((Size().width - textureText_->Size().width) / 2, 0));
				} else {
					renderer_->RenderCopy(textureText_, GUIPoint(5, 0));
				}
			}
			else {
				renderer_->RenderCopy(textureText_, GUIPoint(0, Size().height / 2), angle_, GUIPoint(textureText_->Size().width / 2, textureText_->Size().height / 2));
			}
		}
	} else {
		auto rect = GUIRect(1, Size().height - 29, Size().width-2, 28);
		renderer_->DrawRect(rect, black_color);
		rect = GUIRect(3, Size().height - 27, fullgrad_, 24);
		renderer_->DrawFillRect(rect, own_blue_color);
	}

	needRedraw_ = false;
}

void GUIMediaDisplay::HandleEvent(GUIEvent& event) {
    UNUSED(event);
}

void GUIMediaDisplay::UpdateAnimation() {
	if (!running_) return;

	if(audioManager_ == nullptr || textureMediadata_ == nullptr) return;

	renderer_->RenderTarget(textureMediadata_);

#ifdef ENABLEAUDIOMANAGER
	if (audioManager_->UpdateUi(renderer_, textureMediadata_)) {
		int64_t totalTime;
		int64_t currentTime;
		audioManager_->GetMediaPlayTimes(&totalTime, &currentTime);
		const auto temp = static_cast<double>(currentTime) / static_cast<double>(totalTime);
		fullgrad_ = temp * static_cast<double>((Size().width - 6));
		SetRedraw();
	}
#endif

}

void GUIMediaDisplay::Close()
{
	if(textureText_ != nullptr) {
		delete textureText_;
		textureText_ = nullptr;
	}

	if(textureMediadata_ != nullptr) {
		delete textureMediadata_;
		textureMediadata_ = nullptr;
	}
}

#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "GUITestElement"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif
#include "../../common/easylogging/easylogging++.h"

#include "GUI.h"
#include "GUIElement.h"
#include "GUITestElement.h"
#include "GUIRenderer.h"
#include "GUITexture.h"
#include "GUIImageManager.h"


GUITestElement::GUITestElement(GUIPoint position, GUISize size, const std::string& name):
	GUIElement(position, size, name)
{
	backgroundColor_ = green_color;
	transparency_ = false;
	imageTextureBackground_ = nullptr;
}

GUITestElement::GUITestElement(GUIPoint position, GUISize size, SDL_Color background, const std::string& name):
	GUIElement(position, size, name)
{
	backgroundColor_ = background;
	transparency_ = false;
	imageTextureBackground_ = nullptr;
}

void GUITestElement::Draw() {
	/*if (transparency_) {
		renderer_->DrawLine(0, 0, Size().with, Size().height-1, lightblack_color);
		renderer_->DrawLine(Size().with-1, 0, Size().with-1, Size().height-1, lightblack_color);
	}*/
	if (imageTextureBackground_ != nullptr) {
		auto centerX = Size().width / 2;
		auto centerY = Size().height / 2;
		auto size = imageTextureBackground_->Size();
		auto x = centerX - size.width / 2;
		auto y = centerY - size.height / 2;
		renderer_->RenderCopy(imageTextureBackground_, GUIPoint(x, y));
	}
	needRedraw_ = false;
}

void GUITestElement::HandleEvent(GUIEvent& event)
{
    UNUSED(event);
}

void GUITestElement::UpdateAnimation()
{
}

void GUITestElement::Close()
{

}

void GUITestElement::Transparent() {
	Texture()->SetBlendMode(blendMode::blend);
	Texture()->SetTextureAlphaMod(backgroundColor_.a );
	transparency_ = true;
	needRedraw_ = true;
}

void GUITestElement::Init() {
	//Things after Control is Created
}

void GUITestElement::ImageBackground(const std::string fileName) {
	imageTextureBackground_ = imageManager_->GetImage(fileName);
	SetRedraw();
}
